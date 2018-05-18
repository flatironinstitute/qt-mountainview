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
#ifndef MVSPIKESPRAYPANEL_H
#define MVSPIKESPRAYPANEL_H

#include <QList>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>
#include <QWidget>
#include <mvcontext.h>
#include <functional>

class MVSpikeSprayPanelControlPrivate;
class MVSpikeSprayPanelControl : public QObject {
    Q_OBJECT
    Q_PROPERTY(double amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)
    Q_PROPERTY(double brightness READ brightness WRITE setBrightness NOTIFY brightnessChanged)
    Q_PROPERTY(double weight READ weight WRITE setWeight NOTIFY weightChanged)
    Q_PROPERTY(bool legendVisible READ legendVisible WRITE setLegendVisible NOTIFY legendVisibleChanged)
    Q_PROPERTY(QSet<int> labels READ labels WRITE setLabels NOTIFY labelsChanged)
    Q_PROPERTY(QList<QColor> labelColors READ labelColors WRITE setLabelColors NOTIFY labelColorsChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(int allocationProgress READ allocationProgress NOTIFY allocationProgressChanged)
public:
    MVSpikeSprayPanelControl(QObject* parent = 0);
    ~MVSpikeSprayPanelControl();
    double amplitude() const;
    double brightness() const;
    double weight() const;
    bool legendVisible() const;
    QSet<int> labels() const;

    virtual void paint(QPainter* painter, const QRectF& rect);
    virtual void paintLegend(QPainter* painter, const QRectF& rect);
    const QList<QColor>& labelColors() const;

    void setClips(Mda* X);
    void setRenderLabels(const QVector<int>& X);

    int progress() const;
    int allocationProgress() const;

public slots:
    void setAmplitude(double amplitude);
    void setBrightness(double brightness);
    void setWeight(double weight);
    void setLegendVisible(bool legendVisible);
    void setLabels(const QSet<int>& labels);
    void setLabelColors(const QList<QColor>& labelColors);
    virtual void update(int progress = -1);

signals:
    void brightnessChanged(double brightness);
    void amplitudeChanged(double amplitude);
    void weightChanged(double weight);
    void legendVisibleChanged(bool legendVisible);
    void labelsChanged(QSet<int> labels);
    void labelColorsChanged(QList<QColor> labels);

    void renderAvailable();
    void progressChanged(int);
    void allocationProgressChanged(int);

protected:
    void updateRender();
    void rerender();
private slots:
    void updateAllocProgress(int);

private:
    QSharedPointer<MVSpikeSprayPanelControlPrivate> d;
};

class MVSpikeSprayPanelPrivate;
class MVSpikeSprayPanel : public QWidget {
    Q_OBJECT
public:
    friend class MVSpikeSprayPanelPrivate;
    MVSpikeSprayPanel(MVContext* context);
    virtual ~MVSpikeSprayPanel();
    void setLabelsToUse(const QSet<int>& labels);
    void setClipsToRender(Mda* X);
    void setLabelsToRender(const QVector<int>& X);
    void setAmplitudeFactor(double X);
    void setBrightnessFactor(double factor);
    void setWeightFactor(double factor);
    void setLegendVisible(bool val);

protected:
    void paintEvent(QPaintEvent* evt);

private:
    MVSpikeSprayPanelPrivate* d;
};

class MVSSRenderer : public QObject {
    Q_OBJECT
public:
    Mda clips;
    QList<QColor> colors;
    double amplitude_factor;
    int W;
    int H;
    QPointF coord2pix(int m, double t, double val) const;
    void render_clip(QPainter* painter, int i) const;

    class ClipsAllocator {
    public:
        ClipsAllocator();
        ClipsAllocator(Mda* src, int _M, int _T, const QList<int>& _inds);
        Mda* source = nullptr;
        int M = 0;
        int T = 0;
        QList<int> inds;
        std::function<void(int)> progressFunction;
        Mda allocate(const std::function<bool()>& breakFunc);
    };

    ClipsAllocator allocator;
    void replaceImage(const QImage& img);
    QImage resultImage() const;
    void requestInterruption() { m_int = 1; }
    bool isInterruptionRequested() const { return m_int != 0; }
    void clearInterrupt() { m_int = 0; }
    void wait();
    void release();
public slots:
    void render();
signals:
    void imageUpdated(int progress);
    void allocateProgress(int progress);

private:
    QImage image_in_progress;
    mutable QMutex image_in_progress_mutex;
    QAtomicInteger<bool> m_int = 0;
    QWaitCondition m_cond;
    QMutex m_condMutex;
    bool m_processing = false;
};

#endif // MVSPIKESPRAYPANEL_H
