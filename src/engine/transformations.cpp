#include "engine/transformations.hpp"

auto get_catmull_rom_point(
    float t, Point const p0, Point const p1, Point const p2, Point const p3)
    -> std::pair<Point, Vector> {
    // catmull-rom matrix
    const float m[4][4] = {
        {-0.5f, +1.5f, -1.5f, +0.5f},
        {+1.0f, -2.5f, +2.0f, -0.5f},
        {-0.5f, +0.0f, +0.5f, +0.0f},
        {+0.0f, +1.0f, +0.0f, +0.0f}};

    // point matrix
    const float pm[4][3] = {
        {p0.x(), p0.y(), p0.z()},
        {p1.x(), p1.y(), p1.z()},
        {p2.x(), p2.y(), p2.z()},
        {p3.x(), p3.y(), p3.z()}};

    // a = m X pm
    float a[4][3] = {0};
    for (i32 i = 0; i < 4; i++)
        for (i32 j = 0; j < 3; j++)
            for (i32 k = 0; k < 4; k++) a[i][j] += m[i][k] * pm[k][j];

    // pv = tv X a
    const float tv[4] = {t * t * t, t * t, t, 1};
    float pv[3] = {0};
    for (i32 j = 0; j < 3; j++)
        for (i32 k = 0; k < 4; k++) pv[j] += tv[k] * a[k][j];

    // dv = tvd X a
    const float tvd[4] = {3 * t * t, 2 * t, 1, 0};
    float dv[3] = {0};
    for (i32 j = 0; j < 3; j++)
        for (i32 k = 0; k < 4; k++) dv[j] += tvd[k] * a[k][j];

    return {Point(pv[0], pv[1], pv[2]), Vector(dv[0], dv[1], dv[2])};
}

auto CatmullRon::get_location(float elapsed) const -> std::pair<Point, Vector> {
    const u64 point_count = _points.size(); // number of points
    float gt = elapsed / _time; // how many times has the translation occurred
    float t = gt * point_count; // this is the real global t
    u64 index = std::floor(t);  // which segment
    t = t - index;              // where within  the segment

    u64 indices[4];
    indices[0] = (index + point_count - 1) % point_count;
    indices[1] = (indices[0] + 1) % point_count;
    indices[2] = (indices[1] + 1) % point_count;
    indices[3] = (indices[2] + 1) % point_count;

    return get_catmull_rom_point(
        t,
        _points[indices[0]],
        _points[indices[1]],
        _points[indices[2]],
        _points[indices[3]]);
}

auto build_rotation_matrix(Vector x, Vector y, Vector z) -> std::vector<float> {
    std::vector<float> v;
    v.push_back(x.x());
    v.push_back(x.y());
    v.push_back(x.z());
    v.push_back(0);
    v.push_back(y.x());
    v.push_back(y.y());
    v.push_back(y.z());
    v.push_back(0);
    v.push_back(z.x());
    v.push_back(z.y());
    v.push_back(z.z());
    v.push_back(0);
    v.push_back(0);
    v.push_back(0);
    v.push_back(0);
    v.push_back(1);
    return v;
}

void CatmullRon::apply(bool draw, float elapsed) {
    if (draw) draw_curve();
    auto [point, dir] = get_location(elapsed);
    glTranslatef(point.x(), point.y(), point.z());

    auto X = dir.normalize();
    auto Z = X.cross(Vector(0, 1, 0)).normalize();
    auto Y = Z.cross(X).normalize();

    glMultMatrixf(build_rotation_matrix(X, Y, Z).data());
}

void CatmullRon::draw_curve() const {
    float gt = 0.0;
    const float NUM_STEPS = _points.size() * 20;
    const float gt_step = _time / NUM_STEPS;
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < NUM_STEPS; i++) {
        auto pos_deriv = get_location(gt);
        Point pos = get<0>(pos_deriv);
        glVertex3f(pos.x(), pos.y(), pos.z());
        gt += gt_step;
    }
    glEnd();
}
