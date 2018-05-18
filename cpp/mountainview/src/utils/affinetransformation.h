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

#ifndef AFFINETRANSFORMATION_H
#define AFFINETRANSFORMATION_H

struct Point3D {
    Point3D(float x0 = 0, float y0 = 0, float z0 = 0)
    {
        x = x0;
        y = y0;
        z = z0;
    }

    float x, y, z;
};

class AffineTransformationPrivate;
class AffineTransformation {
public:
    friend class AffineTransformationPrivate;
    AffineTransformation();
    AffineTransformation(const AffineTransformation& other);
    ~AffineTransformation();
    void operator=(const AffineTransformation& other);

    Point3D map(const Point3D& p);

    void setIdentity();
    void rotateX(double theta, bool left = true);
    void rotateY(double theta, bool left = true);
    void rotateZ(double theta, bool left = true);

    void translate(double x, double y, double z, bool left = true);

    void scale(double sx, double sy, double sz, bool left = true);
    void getMatrixData(double* data);

    bool equals(const AffineTransformation& other);

    AffineTransformation inverse() const;

private:
    AffineTransformationPrivate* d;
};

#endif // AFFINETRANSFORMATION_H
