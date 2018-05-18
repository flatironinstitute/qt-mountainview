/*
 * Copyright 2016-2017 Flatiron Institute, Simons Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "mvspikespraypanel.h"

#include <QList>
#include <QMutex>
#include <QPainter>
#include <QPen>
#include <QThread>
#include <QTime>
#include <QTimer>
#include "mda.h"
#include "mlcommon.h"

/*!
 * \class MVSpikeSprayPanelControl
 * \brief The MVSpikeSprayPanelControl class provides a way to render a spike spray panel
 *
 */
/*!
 * \property MVSpikeSprayPanelControl::amplitude
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::brightness
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::weight
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::legendVisible
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::labels
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::labelColors
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::progress
 * \brief
 */
/*!
 * \property MVSpikeSprayPanelControl::allocationProgress
 * \brief
 */

/*!
 * \class MVSSRenderer
 * \brief The MVSSRenderer class represents an object used to perform rendering of a spike spray
 *        panel, likely in a separate thread.
 */

/*!
 * \class MVSSRenderer::ClipsAllocator
 * \brief The ClipsAllocator class performs an allocation and deep copy of Mda structure.
 *
 */

namespace {
/*!
 * \brief A RAII object executes a given function upon destruction
 *
 *        Optionally it also executes another function upon construction.
 */
class RAII {
public:
    using Callback = std::function<void()>;
    RAII(const Callback& func, const Callback& cfunc = Callback())
    {
        m_func = func;
        if (cfunc)
            cfunc();
    }
    ~RAII()
    {
        if (m_func)
            m_func();
    }

private:
    Callback m_func;
};
}

class MVSpikeSprayPanelControlPrivate {
public:
    MVSpikeSprayPanelControlPrivate(MVSpikeSprayPanelControl* qq)
        : q(qq)
    {
    }
    MVSpikeSprayPanelControl* q;
    double amplitude = 1;
    double brightness = 2;
    double weight = 1;
    bool legendVisible = true;
    QSet<int> labels;
    QList<QColor> labelColors;
    Mda* clipsToRender = nullptr;
    QVector<int> renderLabels;
    bool needsRerender = true;
    QThread* renderThread;
    MVSSRenderer* renderer = nullptr;
    QImage render;
    int progress = 0;
    int allocProgress = 0;
};

class MVSpikeSprayPanelPrivate {
public:
    MVSpikeSprayPanel* q;
    MVContext* m_context;
    MVSpikeSprayPanelControl m_control;
};

MVSpikeSprayPanel::MVSpikeSprayPanel(MVContext* context)
{
    d = new MVSpikeSprayPanelPrivate;
    d->q = this;
    d->m_context = context;
    d->m_control.setLabelColors(context->clusterColors());
    // rerender if cluster colors have changed
    QObject::connect(context, SIGNAL(clusterColorsChanged(QList<QColor>)),
        &d->m_control, SLOT(setLabelColors(QList<QColor>)));
    // repaint if new render is available
    connect(&d->m_control, SIGNAL(renderAvailable()), this, SLOT(update()));
    // repaint if progress is updated
    connect(&d->m_control, SIGNAL(progressChanged(int)), this, SLOT(update()));
    // repaint if allocation progress is updated
    connect(&d->m_control, SIGNAL(allocationProgressChanged(int)), this, SLOT(update()));
}

MVSpikeSprayPanel::~MVSpikeSprayPanel()
{
    delete d;
}

void MVSpikeSprayPanel::setLabelsToUse(const QSet<int>& labels)
{
    d->m_control.setLabels(labels);
}

void MVSpikeSprayPanel::setClipsToRender(Mda* X)
{
    d->m_control.setClips(X);
}

void MVSpikeSprayPanel::setLabelsToRender(const QVector<int>& X)
{
    d->m_control.setRenderLabels(X);
}

void MVSpikeSprayPanel::setAmplitudeFactor(double X)
{
    d->m_control.setAmplitude(X);
}

void MVSpikeSprayPanel::setBrightnessFactor(double factor)
{
    d->m_control.setBrightness(factor);
}

void MVSpikeSprayPanel::setWeightFactor(double factor)
{
    d->m_control.setWeight(factor);
}

void MVSpikeSprayPanel::setLegendVisible(bool val)
{
    d->m_control.setLegendVisible(val);
}

void MVSpikeSprayPanel::paintEvent(QPaintEvent* evt)
{
    Q_UNUSED(evt)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    /// TODO (LOW) this should be a configured color to match the cluster view
    painter.fillRect(0, 0, width(), height(), QBrush(QColor(0, 0, 0)));

    d->m_control.paint(&painter, rect());

    // render progress: red stripe for memory allocation, white for rendering
    int progress = d->m_control.progress();
    int allocprogress = d->m_control.allocationProgress();
    if (progress > 0 && progress < 100) {
        QRect r(0, height() - 2, width() * progress * 1.0 / 100, 2);
        painter.fillRect(r, Qt::white);
    }
    else if (!progress && allocprogress > 0 && allocprogress < 100) {
        QRect r(0, height() - 2, width() * allocprogress * 1.0 / 100, 2);
        painter.fillRect(r, Qt::red);
    }
}

QColor brighten(QColor col, double factor)
{
    // TODO: replace with QColor::lighter
    double r = col.red() * factor;
    double g = col.green() * factor;
    double b = col.blue() * factor;
    r = qMin(255.0, r);
    g = qMin(255.0, g);
    b = qMin(255.0, b);
    return QColor(r, g, b);
}

/*!
 * \brief MVSSRenderer::render starts the rendering operation.
 *
 * \warning This method can run for a int period of times.
 */
void MVSSRenderer::render()
{
    RAII releaseRAII([this]() { release(); }); // make sure we call release while returning
    if (isInterruptionRequested()) {
        return; // if we're requested to stop, we bail out here.
    }
    m_processing = true;
    clearInterrupt();
    QImage image = QImage(W, H, QImage::Format_ARGB32); // create an empty image
    image.fill(Qt::transparent); // and make it transparent
    replaceImage(image); // replace the current image
    emit imageUpdated(0); // and report we've just begun
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    // the algorithm is performing lazy memory allocation. This is the last spot we can
    // do the actual allocation (and copy). Bail out if interruption is requested.
    clips = allocator.allocate([this]() { return isInterruptionRequested(); });

    const int L = clips.N3();
    if (L != colors.count()) {
        qWarning() << "Unexpected sizes: " << colors.count() << L;
        return;
    }
    QTime timer;
    timer.start();
    for (int i = 0; i < L; i++) {
        if (timer.elapsed() > 300) { // each 300ms report an intermediate result
            replaceImage(image);
            emit imageUpdated(100 * i / L);
            timer.restart();
        }
        if (isInterruptionRequested()) {
            return;
        }
        render_clip(&painter, i); // render one step
    }
    replaceImage(image); // we're done, expose the result
    emit imageUpdated(100);
}

/*!
 * \brief MVSSRenderer::render_clip performs rendering of a single clip
 *
 */
void MVSSRenderer::render_clip(QPainter* painter, int i) const
{
    QColor col = colors[i];
    const int M = clips.N1();
    const int T = clips.N2();
    const double* ptr = clips.constDataPtr() + (M * T * i);
    QPen pen = painter->pen();
    pen.setColor(col); // set proper color
    painter->setPen(pen);
    for (int m = 0; m < M; m++) {
        QPainterPath path; // we're setting up a painter path
        for (int t = 0; t < T; t++) {
            double val = ptr[m + M * t];
            QPointF pt = coord2pix(m, t, val);
            if (t == 0) {
                path.moveTo(pt);
            }
            else {
                path.lineTo(pt); // plotting a curve
            }
        }
        painter->drawPath(path); // render the complete painter path
    }
}

QPointF MVSSRenderer::coord2pix(int m, double t, double val) const
{
    int M = clips.N1();
    int clip_size = clips.N2();
    double margin_left = 20, margin_right = 20;
    double margin_top = 20, margin_bottom = 20;
    /*
    double max_width = 300;
    if (q->width() - margin_left - margin_right > max_width) {
        double diff = (q->width() - margin_left - margin_right) - max_width;
        margin_left += diff / 2;
        margin_right += diff / 2;
    }
    */
    QRectF rect(margin_left, margin_top, W - margin_left - margin_right, H - margin_top - margin_bottom);
    double pctx = (t + 0.5) / clip_size;
    double pcty = (m + 0.5) / M - val / M * amplitude_factor;
    return QPointF(rect.left() + pctx * rect.width(), rect.top() + pcty * rect.height());
}

MVSpikeSprayPanelControl::MVSpikeSprayPanelControl(QObject* parent)
    : QObject(parent)
    , d(new MVSpikeSprayPanelControlPrivate(this))
{
    d->renderThread = new QThread;
}

MVSpikeSprayPanelControl::~MVSpikeSprayPanelControl()
{
    if (d->renderer) {
        d->renderer->requestInterruption(); // stop the old rendering
        d->renderer->wait(); // and wait for it to die
    }
    d->renderThread->quit();
    if (!d->renderThread->wait(1000 * 10)) {
        qWarning() << "Render thread wouldn't terminate";
    }
    if (d->renderer)
        d->renderer->deleteLater();
    d->renderThread->deleteLater();
}

double MVSpikeSprayPanelControl::amplitude() const
{
    return d->amplitude;
}

void MVSpikeSprayPanelControl::setAmplitude(double amplitude)
{
    if (d->amplitude == amplitude)
        return;
    d->amplitude = amplitude;
    emit amplitudeChanged(amplitude);
    updateRender();
}

double MVSpikeSprayPanelControl::brightness() const
{
    return d->brightness;
}

void MVSpikeSprayPanelControl::setBrightness(double brightness)
{
    if (d->brightness == brightness)
        return;
    d->brightness = brightness;
    emit brightnessChanged(brightness);
    updateRender();
}

double MVSpikeSprayPanelControl::weight() const
{
    return d->weight;
}

void MVSpikeSprayPanelControl::setWeight(double weight)
{
    if (d->weight == weight)
        return;
    d->weight = weight;
    emit weightChanged(weight);
    updateRender();
}

bool MVSpikeSprayPanelControl::legendVisible() const
{
    return d->legendVisible;
}

void MVSpikeSprayPanelControl::setLegendVisible(bool legendVisible)
{
    if (d->legendVisible == legendVisible)
        return;
    d->legendVisible = legendVisible;
    emit legendVisibleChanged(legendVisible);
    update();
}

QSet<int> MVSpikeSprayPanelControl::labels() const
{
    return d->labels;
}

/*!
 * \brief MVSpikeSprayPanelControl::paint renders the panel to a \a painter.
 *
 * The panel is rendered to the given \a rect in \a painter coordinate system.
 * If the panel needs refreshing, a render operation is scheduled. The method will
 * render the current content but will be called again when new content is available.
 *
 * \sa update(), rerender()
 */
void MVSpikeSprayPanelControl::paint(QPainter* painter, const QRectF& rect)
{
    if (d->needsRerender)
        rerender();
    if (!d->render.isNull())
        painter->drawImage(rect, d->render);

    painter->save();
    paintLegend(painter, rect);
    painter->restore();
}

/*!
 * \brief MVSpikeSprayPanelControl::paintLegend renders a view legend to \a painter.
 *
 */
void MVSpikeSprayPanelControl::paintLegend(QPainter* painter, const QRectF& rect)
{
    if (!legendVisible())
        return;

    double W = rect.width();
    double spacing = 6;
    double margin = 10;
    double text_height = qBound(12.0, W * 1.0 / 10, 25.0);
    double y0 = rect.top() + margin;
    QFont font = painter->font();
    font.setPixelSize(text_height - 1);
    painter->setFont(font);

    QSet<int> labels_used = labels();
    QList<int> list = labels_used.toList();
    qSort(list);
    QPen pen = painter->pen();
    for (int i = 0; i < list.count(); i++) {
        QRectF r(rect.left(), y0, W - margin, text_height);
        QString str = QString("%1").arg(list[i]);
        pen.setColor(labelColors().at(list[i]));
        painter->setPen(pen);
        painter->drawText(r, Qt::AlignRight, str);
        y0 += text_height + spacing;
    }
}

void MVSpikeSprayPanelControl::setLabels(const QSet<int>& labels)
{
    if (d->labels == labels)
        return;
    d->labels = labels;
    emit labelsChanged(labels);
    update();
}

const QList<QColor>& MVSpikeSprayPanelControl::labelColors() const
{
    return d->labelColors;
}

/*!
 * \brief setClips sets clip data to be rendered.
 *
 */
void MVSpikeSprayPanelControl::setClips(Mda* X)
{
    if (X == d->clipsToRender)
        return;
    d->clipsToRender = X;
    updateRender();
}

void MVSpikeSprayPanelControl::setRenderLabels(const QVector<int>& X)
{
    if (X == d->renderLabels)
        return;
    d->renderLabels = X;
    updateRender();
}

/*!
 * \brief The progress method returns the current rendering progress
 *
 */
int MVSpikeSprayPanelControl::progress() const
{
    return d->progress;
}

/*!
 * \brief The allocationProgress method returns the current progress of memory allocation
 *
 */
int MVSpikeSprayPanelControl::allocationProgress() const
{
    return d->allocProgress;
}

/*!
 * \brief The setLabelColors method sets colors for the labels.
 *
 */
void MVSpikeSprayPanelControl::setLabelColors(const QList<QColor>& labelColors)
{
    if (labelColors == d->labelColors)
        return;
    d->labelColors = labelColors;
    emit labelColorsChanged(labelColors);
    update();
}

/*!
 * \brief The update method updates the control with the latest image from the renderer.
 *
 * The \a progress argument denotes the current progress reported by the renderer.
 */
void MVSpikeSprayPanelControl::update(int progress)
{
    if (d->renderer) {
        d->render = d->renderer->resultImage(); // get latest image from the renderer
        emit renderAvailable();
    }
    if (progress != -1) {
        d->progress = progress; // if rendering is in progress, report the progress
        emit progressChanged(progress);
    }
}

/*!
 * \brief The updateRender method tells the control a rerender is required.
 */
void MVSpikeSprayPanelControl::updateRender()
{
    d->needsRerender = true;
    update();
}

void MVSpikeSprayPanelControl::rerender()
{
    d->needsRerender = false;
    if (!d->clipsToRender) {
        return; // nothing to render
    }
    if (d->clipsToRender->N3() != d->renderLabels.count()) {
        qWarning() << "Number of clips to render does not match the number of labels to render"
                   << d->clipsToRender->N3() << d->renderLabels.count();
        return; // data mismatch
    }

    if (!amplitude()) { // if amplitude is not given
        double maxval = qMax(qAbs(d->clipsToRender->minimum()), qAbs(d->clipsToRender->maximum()));
        if (maxval)
            d->amplitude = 1.5 / maxval; // try to calculate it automatically
    }

    int K = MLCompute::max(d->renderLabels);
    if (!K) {
        return; // nothing to render
    }
    QVector<int> counts(K + 1, 0);
    QList<int> inds;
    for (int i = 0; i < d->renderLabels.count(); i++) {
        int label0 = d->renderLabels[i];
        if (label0 >= 0) {
            if (this->d->labels.contains(label0)) {
                counts[label0]++;
                inds << i;
            }
        }
    }
    QVector<int> alphas(K + 1);
    for (int k = 0; k <= K; k++) {
        if ((counts[k]) && (weight())) {
            alphas[k] = (int)(255.0 / counts[k] * 2 * weight());
            alphas[k] = qBound(10, alphas[k], 255);
        }
        else
            alphas[k] = 255;
    }

    if (d->renderer) { // since we're chainging the conditions
        d->renderer->requestInterruption(); // stop the old rendering
        d->renderer->wait(); // and wait for it to die
        d->renderer->deleteLater();
        d->renderer = nullptr;
    }
    d->renderer = new MVSSRenderer; // start a new renderer
    if (!d->renderThread->isRunning())
        d->renderThread->start(); // start the thread if not already started

    int M = d->clipsToRender->N1();
    int T = d->clipsToRender->N2();
    // defer copying data until we're in a worker thread to keep GUI responsive
    d->renderer->allocator = { d->clipsToRender, M, T, inds };
    d->renderer->allocator.progressFunction = [this](int progress) {
        emit d->renderer->allocateProgress(progress);
    };

    d->renderer->colors.clear();
    for (int j = 0; j < inds.count(); j++) {
        int label0 = d->renderLabels[inds[j]];
        QColor col = d->labelColors[label0];
        col = brighten(col, brightness());
        if (label0 >= 0) {
            col.setAlpha(alphas[label0]);
        }
        d->renderer->colors << col;
    }

    d->renderer->amplitude_factor = amplitude();
    d->renderer->W = T * 4;
    d->renderer->H = 1500;
    connect(d->renderer, SIGNAL(imageUpdated(int)), this, SLOT(update(int)));
    connect(d->renderer, SIGNAL(allocateProgress(int)), this, SLOT(updateAllocProgress(int)));
    d->renderer->moveToThread(d->renderThread);
    // schedule rendering
    QMetaObject::invokeMethod(d->renderer, "render", Qt::QueuedConnection);
}

void MVSpikeSprayPanelControl::updateAllocProgress(int progress)
{
    if (d->allocProgress == progress)
        return;
    d->allocProgress = progress;
    emit allocationProgressChanged(d->allocProgress);
}

void MVSSRenderer::replaceImage(const QImage& img)
{
    QMutexLocker locker(&image_in_progress_mutex);
    image_in_progress = img;
}

QImage MVSSRenderer::resultImage() const
{
    QMutexLocker locker(&image_in_progress_mutex);
    return image_in_progress;
}

/*!
 * \brief MVSSRenderer::wait method suspends the calling thread until processing is complete.
 */
void MVSSRenderer::wait()
{
    m_condMutex.lock();
    while (m_processing)
        m_cond.wait(&m_condMutex);
    m_condMutex.unlock();
}

/*!
 * \brief MVSSRenderer::release method wakes all threads waiting for processing completion.
 */
void MVSSRenderer::release()
{
    m_condMutex.lock();
    m_processing = false;
    m_cond.wakeAll();
    m_condMutex.unlock();
}

/*!
 * \brief Constructor which creates a no-op allocator
 */
MVSSRenderer::ClipsAllocator::ClipsAllocator() {}

/*!
 * \brief Constructor which initializes the allocator
 * \param src Mda structure to be copied
 * \param _M 1st dim
 * \param _T 2nd dim
 * \param _inds 3rd dim
 */
MVSSRenderer::ClipsAllocator::ClipsAllocator(Mda* src, int _M, int _T, const QList<int>& _inds)
    : source(src)
    , M(_M)
    , T(_T)
    , inds(_inds)
{
}

Mda MVSSRenderer::ClipsAllocator::allocate(const std::function<bool()>& breakFunc)
{
    Mda result(M, T, inds.count());
    for (int j = 0; j < inds.count(); j++) {
        for (int t = 0; t < T; t++) {
            for (int m = 0; m < M; m++) {
                result.setValue(source->value(m, t, inds[j]), m, t, j);
            }
        }
        if (breakFunc && breakFunc())
            return result; // bail out if requested
        if (progressFunction)
            progressFunction(j * 100 / inds.count()); // report progress
    }
    if (progressFunction)
        progressFunction(100); // report completion
    return result;
}
