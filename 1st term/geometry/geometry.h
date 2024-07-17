#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

// НА РЕВЬЮ

const double kEpsilon = 1e-7;
const double kPi = std::acos(-1);

bool Equal(double first_num, double second_num) {
  return std::abs(first_num - second_num) < kEpsilon;
}

int Sign(double num) {
  if (Equal(num, 0)) {
    return 0;
  }

  if (num < 0) {
    return -1;
  }

  return 1;
}

bool NotEqual(double first_num, double second_num) {
  return !Equal(first_num, second_num);
}

struct Point {
  double x = 0;
  double y = 0;

  Point operator = (Point point) {
    x = point.x;
    y = point.y;
    return *this;
  }

  Point& operator += (const Point& other){
    x += other.x;
    y += other.y;
    return *this;
  }

  Point operator - (){
    return Point{-x, -y};
  }

  Point() = default;
  Point(double x, double y) : x(x), y(y) {}
  Point(std::pair<double, double> coords) : Point(coords.first, coords.second) {}
  Point(const Point& other) : Point(other.x, other.y) {}
};

bool operator == (const Point& first, const Point& second) {
  return Equal(first.x, second.x) && Equal(first.y, second.y);
}

Point operator + (const Point& first, const Point& second) {
  Point temp(first);
  temp += second;
  return temp;
}

bool operator != (const Point& first, const Point& second) {
  return !(first == second);
}

double points_distance(const Point& first, const Point& second) {
  double x_offset = first.x - second.x;
  double y_offset = first.y - second.y;

  return std::sqrt(x_offset * x_offset + y_offset * y_offset);
}

struct Vector {
  double x = 0;
  double y = 0;

  Vector() = default;
  Vector(double x, double y) : x(x), y(y) {}
  Vector(std::pair<double, double> coords) : Vector(coords.first, coords.second) {}
  Vector(const Point& point) : Vector(point.x, point.y) {}
  Vector(const Point& start, const Point& end) : Vector(end.x - start.x, end.y - start.y) {}

  double length() const {
    return std::sqrt(x * x + y * y);
  }
};

double dot_product(const Vector& first, const Vector& second) {
  return first.x * second.x + first.y * second.y;
}

double cross_product(const Vector& first, const Vector& second) {
  return first.x * second.y - first.y * second.x;
}

double triangle_area(const Point& first, const Point& second, const Point& third) {
  Vector first_vector{first, second};
  Vector second_vector{first, third};

  return std::abs(cross_product(first_vector, second_vector)) / 2.0;
}

class Line {
 private:
  double coef_ = 1;
  double shift_ = 0; // intersection with oY if not vertical, intersection with oX if vertical
  bool is_vertical_ = false;

 public:
  double coef() const {
    return coef_;
  }

  double shift() const {
    return shift_;
  }

  bool is_vertical() const {
    return is_vertical_;
  }

  Line() = default;
  Line(const Point& first, const Point& second) {
    if (Equal(first.x, second.x)) {
      is_vertical_ = true;
      shift_ = first.x;
    } else {
      coef_ = (second.y - first.y) / (second.x - first.x);
      shift_ = first.y - coef_ * first.x;
    }
  }
  Line(double coef, double shift) : coef_(coef), shift_(shift) {}
  Line(Point point, double coef) : coef_(coef) {
    shift_ = point.y - coef_ * point.x;
  }
};

bool operator == (const Line& first, const Line& second) {
  return Equal(first.coef(), second.coef()) && Equal(first.shift(), second.shift()) &&
         Equal(first.is_vertical(), second.is_vertical());
}

bool operator != (const Line& first, const Line& second) {
  return !(first == second);
}

Point intersect_lines(const Line& first, const Line& second) {
  double result_x = 0;
  double result_y = 0;

  if (first.is_vertical()) {
    result_x = first.shift();
    result_y = second.coef() * result_x + second.shift();
  } else if (second.is_vertical()) {
    result_x = second.shift();
    result_y = first.coef() * result_x + first.shift();
  } else {
    result_x = (second.shift() - first.shift()) / (first.coef() - second.coef());
    result_y = first.coef() * result_x + first.shift();
  }

  return Point{result_x, result_y};
}

Line perpendicular_line_through_point(const Line& line, const Point& point) {
  if (line.is_vertical()) {
    return Line{0, point.y};
  }

  if (Equal(line.coef(), 0)) {
    return Line{point, Point{point.x, point.y + 1}};
  }

  return Line{point, -1 / line.coef()};
}

Point rotate_point(const Point& center, const Point& point, double angle) {
  double new_x = (point.x - center.x) * std::cos(angle) - (point.y - center.y) * std::sin(angle) + center.x;
  double new_y = (point.x - center.x) * std::sin(angle) + (point.y - center.y) * std::cos(angle) + center.y;

  return Point{new_x, new_y};
}

Point reflect_point(const Point& center, const Point& point) {
  double new_x = 2 * center.x - point.x;
  double new_y = 2 * center.y - point.y;

  return Point{new_x, new_y};
}

Point reflect_point(const Line& line, const Point& point) {
  Line perp = perpendicular_line_through_point(line, point);
  Point projection = intersect_lines(line, perp);

  return reflect_point(projection, point);
}

Point scale_point(const Point& center, const Point& point, double coef) {
  double new_x = center.x + (point.x - center.x) * coef;
  double new_y = center.y + (point.y - center.y) * coef;

  return Point{new_x, new_y};
}

bool is_point_on_line(const Line& line, const Point& point) {
  return (line.is_vertical() && Equal(point.x, line.shift())) ||
          Equal(line.coef() * point.x + line.shift(), point.y);
}

bool is_point_on_segment(const Point& point, const Point& seg_start, const Point& seg_end) {
  Line line{seg_start, seg_end};

  if (!is_point_on_line(line, point)) {
    return false;
  }

  double min_x = std::min(seg_start.x, seg_end.x);
  double max_x = std::max(seg_start.x, seg_end.x);
  double min_y = std::min(seg_start.y, seg_end.y);
  double max_y = std::max(seg_start.y, seg_end.y);

  return (min_x <= point.x && point.x <= max_x && min_y <= point.y && point.y <= max_y);
}

bool do_ray_and_segment_intersect(const Point& start, const Point& end, const Point& ray_start,
                                  const Vector& ray_vector) {
  Line first_line{start, end};
  Line second_line{ray_start, Point{ray_start.x + ray_vector.x, ray_start.y + ray_vector.y}};
  Point point = intersect_lines(first_line, second_line);

  return Sign(point.x - ray_start.x) == Sign(ray_vector.x) &&
         is_point_on_segment(point, start, end) &&
         is_point_on_line(second_line, point);
}

Point midpoint(const Point& first, const Point& second) {
  return Point{(first.x + second.x) / 2, (first.y + second.y) / 2};
}

Line perpendicular_bisector(const Point& first, const Point& second) {
  return perpendicular_line_through_point(
    Line{first, second},
    midpoint(first, second)
  );
}

class Shape {
 public:
  virtual bool is_polygon() const = 0;
  virtual bool is_ellipse() const = 0;
  virtual bool is_circle() const = 0;

  virtual double perimeter() const = 0;
  virtual double area() const = 0;
  virtual bool isSimilarTo(const Shape& another) const = 0;
  virtual bool containsPoint(const Point& point) const = 0;

  virtual void rotate(const Point& center, double angle) = 0;
  virtual void reflect(const Point& center) = 0;
  virtual void reflect(const Line& axis) = 0;
  virtual void scale(const Point& center, double coefficient) = 0;

  bool isCongruentTo(const Shape& another) const {
    return isSimilarTo(another) && Equal(area(), another.area());
  }

  virtual ~Shape() = default;
};

class Polygon : public Shape {
 protected:
  std::vector<Point> vertices_;

 private:
  double getAngle(size_t vertex_index) const {
    size_t size = verticesCount();

    Vector first{vertices_[vertex_index], vertices_[(vertex_index + size - 1) % size]};
    Vector second{vertices_[vertex_index], vertices_[(vertex_index + 1) % size]};

    return std::acos(dot_product(first, second) / first.length() / second.length());
  }

  std::vector<double> getAngles() const {
    size_t size = verticesCount();
    std::vector<double> result(size);
    
    for (size_t i = 0; i < size; ++i) {
      result[i] = getAngle(i);
    }

    return result;
  }

  std::vector<double> getSides() const {
    size_t size = verticesCount();
    std::vector<double> result(size);

    for (size_t i = 0; i < size; ++i) {
      result[i] = points_distance(vertices_[i], vertices_[(i + 1) % size]);
    }

    return result;
  }

 public:
  bool is_polygon() const override { return true; }
  bool is_ellipse() const override { return false; }
  bool is_circle() const override { return false; }

  size_t verticesCount() const {
    return vertices_.size();
  }

  const std::vector<Point> getVertices() const {
    return vertices_;
  }

  bool isConvex() const {
    int sign = Sign(cross_product(Vector{vertices_[1], vertices_[0]}, Vector{vertices_[2], vertices_[0]}));
    size_t size = vertices_.size();

    for (size_t i = 0; i < size; ++i) {
      int new_sign = Sign(cross_product(Vector{vertices_[(i + 1) % size], vertices_[i]}, Vector{vertices_[(i + 2) % size], vertices_[i]}));

      if (sign != new_sign) {
        return false;
      }
    }
    
    return true;
  }
  
  double perimeter() const override {
    double answer = 0;
    
    for (size_t i = 0; i < vertices_.size(); ++i) {
      answer += points_distance(vertices_[i], vertices_[(i + 1) % vertices_.size()]);
    }

    return answer;
  }

  double area() const override {
    double answer = 0;

    for (size_t i = 0; i < vertices_.size(); ++i) {
      Vector first{vertices_[i]};
      Vector second{vertices_[(i + 1) % vertices_.size()]};

      answer += cross_product(first, second);
    }

    return std::abs(answer) / 2;
  }

  bool operator == (const Shape& other) const {
    if (!other.is_polygon()) {
      return false;
    }

    const Polygon& new_other = dynamic_cast<const Polygon&>(other);

    if (new_other.verticesCount() != verticesCount()) {
      return false;
    }

    size_t size = vertices_.size();
    std::vector<Point> other_vertices;
    other_vertices.assign(new_other.vertices_.begin(), new_other.vertices_.end());

    for (int iter = 0; iter < 2; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        bool found = true;

        for (size_t j = 0; j < size; ++j) {
          if (vertices_[j] != other_vertices[(i + j) % size]) {
            found = false;
            break;
          }
        }

        if (found == true) {
          return true;
        }
      }

      if (iter == 0) {
        std::reverse(other_vertices.begin(), other_vertices.end());
      }
    }

    return false;
  }

  bool operator != (const Shape& another) const {
    return !(*this == another);
  }

  bool isSimilarTo(const Shape& other) const override {
    if (!other.is_polygon()) {
      return false;
    }

    const Polygon& new_other = dynamic_cast<const Polygon&>(other);

    if (new_other.verticesCount() != verticesCount()) {
      return false;
    }

    std::vector<double> angles = getAngles();
    std::vector<double> other_angles = new_other.getAngles();

    size_t size = vertices_.size();

    for (int iter = 0; iter < 2; ++iter) {
      for (size_t i = 0; i < size; ++i) {
        bool found = true;

        for (size_t j = 0; j < size; ++j) {
          if (NotEqual(angles[j], other_angles[(i + j) % size])) {
            found = false;
            break;
          }
        }

        if (found == true) {
          return true;
        }
      }

      if (iter == 0) {
        std::reverse(other_angles.begin(), other_angles.end());
      }
    }

    return false;
  }

  bool containsPoint(const Point& point) const override {
    size_t intersections = 0;

    for (size_t i = 0; i < vertices_.size(); ++i) {
      if (is_point_on_segment(point, vertices_[i], vertices_[(i + 1) % vertices_.size()])) {
        return true;
      }

      if (do_ray_and_segment_intersect(vertices_[i], vertices_[(i + 1) % vertices_.size()], point, Vector{1, kPi})) {
        ++intersections;
      }
    }

    return (intersections % 2 == 1);
  }

  void rotate(const Point& center, double angle) override {
    for (size_t i = 0; i < vertices_.size(); ++i) {
      vertices_[i] = rotate_point(center, vertices_[i], angle);
    }
  }

  void reflect(const Point& center) override {
    for (size_t i = 0; i < vertices_.size(); ++i) {
      vertices_[i] = reflect_point(center, vertices_[i]);
    }
  }

  void reflect(const Line& line) override {
    for (size_t i = 0; i < vertices_.size(); ++i) {
      vertices_[i] = reflect_point(line, vertices_[i]);
    }
  }

  void scale(const Point& center, double coefficient) override {
    for (size_t i = 0; i < vertices_.size(); ++i) {
      vertices_[i] = scale_point(center, vertices_[i], coefficient);
    }
  }

  Polygon() = default;
  Polygon(std::vector<Point> vertices) : vertices_(vertices) {}
  Polygon(Point point) {
    vertices_.push_back(point);
  }
  template<typename... T>
  Polygon(const T&... args) {
    for (auto&& point : std::initializer_list<Point>{args...}) {
      vertices_.push_back(point);
    }
  }
};

class Ellipse : public Shape {
 private:
  Point first_focus_;
  Point second_focus_;
  double distance_sum_ = 0;
  double focal_distance_ = 0;
  double small_half_axis_ = 0;
  double big_half_axis_ = 0;

 public:
  bool is_polygon() const override { return false; }
  bool is_ellipse() const override { return true; }
  bool is_circle() const override { return false; }

  std::pair<Point, Point> focuses() const {
    return {first_focus_, second_focus_};
  }

  std::pair<Line, Line> directrices() const {
    double new_x_1 = first_focus_.x + (second_focus_.x - first_focus_.x) /
           (2 * focal_distance_) *
           (big_half_axis_ * big_half_axis_ / focal_distance_ + focal_distance_);
    double new_y_1 = first_focus_.y + (second_focus_.y - first_focus_.y) /
           (2 * focal_distance_) *
           (big_half_axis_ * big_half_axis_ / focal_distance_ + focal_distance_);
    double new_x_2 = second_focus_.x + (first_focus_.x - second_focus_.x) /
           (2 * focal_distance_) *
           (big_half_axis_ * big_half_axis_ / focal_distance_ + focal_distance_);
    double new_y_2 = second_focus_.y + (first_focus_.y - second_focus_.y) /
           (2 * focal_distance_) *
           (big_half_axis_ * big_half_axis_ / focal_distance_ + focal_distance_);

    Point first_new_point{new_x_1, new_y_1};
    Point second_new_point{new_x_2, new_y_2};

    Line symmetry{first_focus_, second_focus_};
    Line first_directrice = perpendicular_line_through_point(symmetry, first_new_point);
    Line second_directrice = perpendicular_line_through_point(symmetry, second_new_point);

    return {first_directrice, second_directrice};
  }

  double eccentricity() const {
    return focal_distance_ / big_half_axis_;
  }

  Point center() const {
    return midpoint(first_focus_, second_focus_);
  }

  double perimeter() const override {
    double ratio = (big_half_axis_ - small_half_axis_) / (big_half_axis_ + small_half_axis_);

    return kPi * (small_half_axis_ + big_half_axis_) *
           (1 + (3 * ratio * ratio) / (10 + std::sqrt(4 - 3 * ratio * ratio)));
  }

  double area() const override {
    return kPi * small_half_axis_ * big_half_axis_;
  }

  bool operator == (const Shape& other) const {
    if (!other.is_ellipse()) {
      return false;
    }

    const Ellipse& new_other = dynamic_cast<const Ellipse&>(other);

    return (first_focus_ == new_other.first_focus_ &&
            second_focus_ == new_other.second_focus_ &&
            Equal(distance_sum_, new_other.distance_sum_)) ||
            (first_focus_ == new_other.second_focus_ &&
            second_focus_ == new_other.first_focus_ &&
            Equal(distance_sum_, new_other.distance_sum_));
  }

  bool operator != (const Shape& another) const {
    return !(*this == another);
  }

  bool isSimilarTo(const Shape& other) const override {
    if (!other.is_ellipse()) {
      return false;
    }

    const Ellipse& new_other = dynamic_cast<const Ellipse&>(other);

    return Equal(eccentricity(), new_other.eccentricity());
  }

  bool containsPoint(const Point& point) const override {
    return points_distance(point, first_focus_) + points_distance(point, second_focus_) <
           distance_sum_;
  }

  void rotate(const Point& center, double angle) override {
    first_focus_ = rotate_point(center, first_focus_, angle);
    second_focus_ = rotate_point(center, second_focus_, angle);
  }

  void reflect(const Point& center) override {
    first_focus_ = reflect_point(center, first_focus_);
    second_focus_ = reflect_point(center, second_focus_);
  }

  void reflect(const Line& line) override {
    first_focus_ = reflect_point(line, first_focus_);
    second_focus_ = reflect_point(line, second_focus_);
  }

  void scale(const Point& center, double coefficient) override {
    first_focus_ = scale_point(center, first_focus_, coefficient);
    second_focus_ = scale_point(center, second_focus_, coefficient);
    distance_sum_ *= coefficient;
    focal_distance_ *= coefficient;
    small_half_axis_ *= coefficient;
    big_half_axis_ *= coefficient;
  }

  Ellipse() = default;
  Ellipse(Point first_focus, Point second_focus, double distance_sum) : 
          first_focus_(first_focus), second_focus_(second_focus), distance_sum_(distance_sum) {
            focal_distance_ = points_distance(first_focus_, second_focus_) / 2;
            small_half_axis_ = std::sqrt(distance_sum_ * distance_sum_ -
                                        4 * focal_distance_ * focal_distance_) / 2;
            big_half_axis_ = distance_sum_ / 2;
          }
};

class Circle : public Shape {
 private:
  Point center_;
  double radius_ = 0;

 public:
  bool is_polygon() const override { return false; }
  bool is_ellipse() const override { return false; }
  bool is_circle() const override { return true; }

  double radius() const {
    return radius_;
  }

  const Point center() const {
    return center_;
  }

  double perimeter() const override {
    return 2 * kPi * radius_;
  }

  double area() const override {
    return kPi * radius_ * radius_;
  }

  bool operator == (const Shape& other) const {
    if (!other.is_circle()) {
      return false;
    }

    const Circle& new_other = dynamic_cast<const Circle&>(other);

    return Equal(new_other.radius_, radius_) && new_other.center_ == center_;
  }

  bool operator != (const Shape& another) const {
    return !(*this == another);
  }

  bool isSimilarTo(const Shape& other) const override {
    return other.is_circle();
  }

  bool containsPoint(const Point& point) const override {
    return points_distance(point, center_) <= radius_;
  }

  void rotate(const Point& center, double angle) override {
    center_ = rotate_point(center, center_, angle);
  }

  void reflect(const Point& center) override {
    center_ = reflect_point(center, center_);
  }

  void reflect(const Line& line) override {
    center_ = reflect_point(line, center_);
  }

  void scale(const Point& center, double coef) override {
    center_ = scale_point(center, center_, coef);
    radius_ *= coef;
  }

  Circle() = default;
  Circle(const Point& center, double radius) : center_(center), radius_(radius) {}
  Circle(const Point& first, const Point& second, const Point& third) {
    Line first_line = perpendicular_bisector(first, second);
    Line second_line = perpendicular_bisector(first, third);

    center_ = intersect_lines(first_line, second_line);
    radius_ = points_distance(center_, first);
  }
};

class Rectangle : public Polygon {
 public:
  Point center() const {
    return midpoint(vertices_[0], vertices_[2]);
  }

  std::pair<Line, Line> diagonals() const {
    return {Line{vertices_[0], vertices_[2]},
            Line{vertices_[1], vertices_[3]}};
  }

  Rectangle() = default;
  Rectangle(Point first_corner, Point second_corner, double ratio) {
    if (ratio > 1) {
      ratio = 1 / ratio;
    }

    double diag_len = points_distance(first_corner, second_corner);
    double second_side_square = (diag_len * diag_len) / (ratio * ratio + 1);
    double first_side_square = ratio * ratio * second_side_square;

    double first_part = 0.5 * ((first_side_square - second_side_square) / diag_len + diag_len);
    double second_part = diag_len - first_part;

    double dist = std::sqrt(first_side_square - first_part * first_part);

    double div_x = first_corner.x + (second_corner.x - first_corner.x) *
                   first_part / (first_part + second_part);
    double div_y = first_corner.y + (second_corner.y - first_corner.y) *
                   first_part / (first_part + second_part);

    double new_x_1 = div_x + (first_corner.y - div_y) * dist / first_part;
    double new_y_1 = div_y + (div_x - first_corner.x) * dist / first_part;
    double new_x_2 = div_x - (first_corner.y - div_y) * dist / first_part;
    double new_y_2 = div_y - (div_x - first_corner.x) * dist / first_part;

    Point first_candidate{new_x_1, new_y_1};
    Point second_candidate{new_x_2, new_y_2};

    Vector first_vec{first_corner, first_candidate};
    Vector second_vec{first_corner, second_candidate};
    Vector diagonal{first_corner, second_corner};

    Point second_vertex;

    if (dot_product(diagonal, first_vec) > 0) {
      second_vertex = first_candidate;
    } else {
      second_vertex = second_candidate;
    }

    vertices_.push_back(first_corner);
    vertices_.push_back(second_vertex);
    vertices_.push_back(second_corner);
    vertices_.push_back(reflect_point(midpoint(first_corner, second_corner), second_vertex));
  }
};

class Square : public Rectangle {
 private:
  double side() const {
    return points_distance(vertices_[0], vertices_[1]);
  }

 public:
  Circle circumscribedCircle() const {
    return Circle{center(), std::sqrt(2) * side()};
  }

  Circle inscribedCircle() const {
    return Circle{center(), side() / 2};
  }

  Square() = default;
  Square(const Point& first, const Point& second) : Rectangle(first,  second, 1) {}
};

class Triangle : public Polygon {
 public:
  Circle circumscribedCircle() const {
    return Circle{vertices_[0], vertices_[1], vertices_[2]};
  }

  Circle inscribedCircle() const {
    double side1 = points_distance(vertices_[0], vertices_[1]);
    double side2 = points_distance(vertices_[1], vertices_[2]);
    double side3 = points_distance(vertices_[2], vertices_[0]);
    double perimeter = side1 + side2 + side3;

    Point center{
      side1 / perimeter * (vertices_[2].x - vertices_[0].x) +
      side3 / perimeter * (vertices_[1].x - vertices_[0].x) + vertices_[0].x,
      side1 / perimeter * (vertices_[2].y - vertices_[0].y) +
      side3 / perimeter * (vertices_[1].y - vertices_[0].y) + vertices_[0].y
    };
    double radius = 2 * area() / perimeter;

    return Circle{center, radius};
  }

  Point centroid() const {
    return Point{
      (vertices_[0].x + vertices_[1].x + vertices_[2].x) / 3,
      (vertices_[0].y + vertices_[1].y + vertices_[2].y) / 3
    };
  }

  Point orthocenter() const {
    Line first_side{vertices_[0], vertices_[1]};
    Line second_side{vertices_[0], vertices_[2]};

    Line first_height = perpendicular_line_through_point(first_side, vertices_[2]);
    Line second_height = perpendicular_line_through_point(second_side, vertices_[1]);

    return intersect_lines(first_height, second_height);
  }

  Line EulerLine() const {
    Point first = circumscribedCircle().center();
    Point second = orthocenter();

    return Line{first, second};
  }

  Circle ninePointsCircle() const {
    return Circle{
      midpoint(vertices_[0], vertices_[1]),
      midpoint(vertices_[0], vertices_[2]),
      midpoint(vertices_[1], vertices_[2])
    };
  }

  Triangle() = default;
  Triangle(const Point& first, const Point& second, const Point& third) :
           Polygon(first, second, third) {}
};

bool operator == (const Shape& first, const Shape& second) {
  if (first.is_polygon()) {
    const Polygon& new_first = dynamic_cast<const Polygon&>(first);
    return new_first == second;
  } else if (first.is_ellipse()) {
    const Ellipse& new_first = dynamic_cast<const Ellipse&>(first);
    return new_first == second;
  } else if (first.is_circle()) {
    const Circle& new_first = dynamic_cast<const Circle&>(first);
    return new_first == second;
  }

  return false;
}

bool operator != (const Shape& first, const Shape& second) {
  if (first.is_polygon()) {
    const Polygon& new_first = dynamic_cast<const Polygon&>(first);
    return new_first != second;
  } else if (first.is_ellipse()) {
    const Ellipse& new_first = dynamic_cast<const Ellipse&>(first);
    return new_first != second;
  } else if (first.is_circle()) {
    const Circle& new_first = dynamic_cast<const Circle&>(first);
    return new_first != second;
  }

  return false;
}
