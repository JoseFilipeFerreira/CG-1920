#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include "utils/point.hpp"
#include "utils/types.hpp"
#include "utils/visitor.hpp"

#include <array>
#include <utility>
#include <vector>

#ifdef __APPLE__
#    include <GLUT/glut.h>
#else
#    include <GL/glew.h>
#    include <GL/glut.h>
#endif

class Colour {
  private:
    float _r, _g, _b, _a;

  public:
    Colour(): _r(1), _g(1), _b(1), _a(1) {}
    Colour(float r, float g, float b, float a): _r(r), _g(g), _b(b), _a(a) {}
    auto constexpr r() noexcept -> float { return _r; }
    auto constexpr g() noexcept -> float { return _g; }
    auto constexpr b() noexcept -> float { return _b; }
    auto constexpr a() noexcept -> float { return _a; }
    void apply() const { glColor4f(_r, _g, _b, _a); }
    void set_diffuse() const {
        float a[] = {_r, _g, _b, _a};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, a);
    }
    void set_specular() const {
        float a[] = {_r, _g, _b, _a};
        glMaterialfv(GL_FRONT, GL_SPECULAR, a);
    }
    void set_emissive() const {
        float a[] = {_r, _g, _b, _a};
        glMaterialfv(GL_FRONT, GL_EMISSION, a);
    }
    void set_ambient() const {
        float a[] = {_r, _g, _b, _a};
        glMaterialfv(GL_FRONT, GL_AMBIENT, a);
    }
};

class Scale {
  private:
    float _x, _y, _z;

  public:
    Scale(float x, float y, float z): _x(x), _y(y), _z(z) {}
    void apply(bool, float elapsed) const { glScalef(_x, _y, _z); }
};

class Translate {
  private:
    float _x, _y, _z;

  public:
    Translate(float x, float y, float z): _x(x), _y(y), _z(z) {}
    void apply(bool, float elapsed) const { glTranslatef(_x, _y, _z); }
};

class CatmullRon {
  private:
    float _time;
    std::vector<Point> _points;

  public:
    CatmullRon(float time, std::vector<Point> points)
        : _time(time), _points(points) {}
    void apply(bool, float);
    auto get_location(float) const -> std::pair<Point, Vector>;
    void draw_curve() const;
};

class Rotate {
  private:
    float _ang, _x, _y, _z, _time;

  public:
    Rotate(float ang, float x, float y, float z, float time)
        : _ang(ang), _x(x), _y(y), _z(z), _time(time) {}
    void apply(bool, float elapsed) const {
        float total_ang = _ang + elapsed * (_time ? 360.0f / _time : 0);
        glRotatef(total_ang, _x, _y, _z);
    }
};

class Transform {
  private:
    std::variant<Scale, Translate, CatmullRon, Rotate> _t;

  public:
    Transform(Scale t): _t(t){};
    Transform(Translate t): _t(t){};
    Transform(CatmullRon t): _t(t){};
    Transform(Rotate t): _t(t){};

    void apply(bool draw, float elapsed) const {
        std::visit(
            overloaded{
                [&](Scale t) { t.apply(draw, elapsed); },
                [&](Translate t) { t.apply(draw, elapsed); },
                [&](CatmullRon t) { t.apply(draw, elapsed); },
                [&](Rotate t) { t.apply(draw, elapsed); },
            },
            _t);
    }
};

#endif // TRANSFORMATIONS_H
