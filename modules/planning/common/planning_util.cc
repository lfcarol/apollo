/******************************************************************************
 * Copyright 2017 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "modules/planning/common/planning_util.h"

#include <array>
#include <cmath>
#include <memory>
#include <utility>

#include "modules/common/adapters/adapter_manager.h"
#include "modules/common/math/integral.h"
#include "modules/common/math/linear_interpolation.h"
#include "modules/common/math/math_utils.h"
#include "modules/planning/common/planning_gflags.h"
#include "modules/planning/math/hermite_spline.h"

namespace apollo {
namespace planning {
namespace util {

using common::PathPoint;
using common::SpeedPoint;
using common::TrajectoryPoint;
using common::adapter::AdapterManager;

PathPoint interpolate(const PathPoint &p0, const PathPoint &p1,
                      const double s) {
  double s0 = p0.s();
  double s1 = p1.s();
  CHECK(s0 <= s && s <= s1);

  double theta_diff = common::math::NormalizeAngle(p1.theta() - p0.theta());

  std::array<double, 3> gx0{{0.0, p0.kappa(), p0.dkappa()}};
  std::array<double, 3> gx1{{theta_diff, p1.kappa(), p1.dkappa()}};

  HermiteSpline<double, 5> geometry_spline(gx0, gx1, s0, s1);
  auto func_cos_theta = [&geometry_spline, &p0](const double s) {
    auto theta = geometry_spline.Evaluate(0, s) + p0.theta();
    return std::cos(theta);
  };
  auto func_sin_theta = [&geometry_spline, &p0](const double s) {
    auto theta = geometry_spline.Evaluate(0, s) + p0.theta();
    return std::sin(theta);
  };

  double x =
      p0.x() + common::math::IntegrateByGaussLegendre<5>(func_cos_theta, s0, s);
  double y =
      p0.y() + common::math::IntegrateByGaussLegendre<5>(func_sin_theta, s0, s);
  double theta =
      common::math::NormalizeAngle(geometry_spline.Evaluate(0, s) + p0.theta());
  double kappa = geometry_spline.Evaluate(1, s);
  double dkappa = geometry_spline.Evaluate(2, s);
  double d2kappa = geometry_spline.Evaluate(3, s);

  PathPoint p;
  p.set_x(x);
  p.set_y(y);
  p.set_theta(theta);
  p.set_kappa(kappa);
  p.set_dkappa(dkappa);
  p.set_ddkappa(d2kappa);
  p.set_s(s);
  return p;
}

TrajectoryPoint interpolate(const TrajectoryPoint &tp0,
                            const TrajectoryPoint &tp1, const double t) {
  if (std::abs(tp0.path_point().s() - tp0.path_point().s()) < 1.0e-4) {
    return tp1;
  }

  const PathPoint &pp0 = tp0.path_point();
  const PathPoint &pp1 = tp1.path_point();
  double t0 = tp0.relative_time();
  double t1 = tp1.relative_time();

  std::array<double, 2> dx0{{tp0.v(), tp0.a()}};
  std::array<double, 2> dx1{{tp1.v(), tp1.a()}};
  HermiteSpline<double, 3> dynamic_spline(dx0, dx1, t0, t1);

  double s0 = 0.0;
  auto func_v = [&dynamic_spline](const double t) {
    return dynamic_spline.Evaluate(0, t);
  };
  double s1 = common::math::IntegrateByGaussLegendre<5>(func_v, t0, t1);
  double s = common::math::IntegrateByGaussLegendre<5>(func_v, t0, t);

  if (std::abs(tp0.path_point().s() - s1) < 1.0e-4) {
    return tp1;
  }

  double v = dynamic_spline.Evaluate(0, t);
  double a = dynamic_spline.Evaluate(1, t);

  std::array<double, 2> gx0{{pp0.theta(), pp0.kappa()}};
  std::array<double, 2> gx1{{pp1.theta(), pp1.kappa()}};
  HermiteSpline<double, 3> geometry_spline(gx0, gx1, s0, s1);
  auto func_cos_theta = [&geometry_spline](const double s) {
    auto theta = geometry_spline.Evaluate(0, s);
    return std::cos(theta);
  };
  auto func_sin_theta = [&geometry_spline](const double s) {
    auto theta = geometry_spline.Evaluate(0, s);
    return std::sin(theta);
  };

  double x = pp0.x() +
             common::math::IntegrateByGaussLegendre<5>(func_cos_theta, s0, s);
  double y = pp0.y() +
             common::math::IntegrateByGaussLegendre<5>(func_sin_theta, s0, s);
  double theta = geometry_spline.Evaluate(0, s);
  double kappa = geometry_spline.Evaluate(1, s);
  double dkappa = geometry_spline.Evaluate(2, s);
  double d2kappa = geometry_spline.Evaluate(3, s);

  TrajectoryPoint tp;
  tp.set_v(v);
  tp.set_a(a);

  PathPoint *path_point = tp.mutable_path_point();
  path_point->set_x(x);
  path_point->set_y(y);
  path_point->set_theta(theta);
  path_point->set_kappa(kappa);
  path_point->set_dkappa(dkappa);
  path_point->set_ddkappa(d2kappa);
  path_point->set_s(s);

  // check the diff of computed s1 and p1.s()?
  return tp;
}

common::SLPoint interpolate(const common::SLPoint &start,
                            const common::SLPoint &end, const double weight) {
  common::SLPoint point;
  double s = start.s() * (1 - weight) + end.s() * weight;
  double l = start.l() * (1 - weight) + end.l() * weight;
  point.set_s(s);
  point.set_l(l);
  return point;
}

PlanningState *GetPlanningState() {
  static PlanningState state;
  return &state;
}

void DumpPlanningContext() {
  AdapterManager::GetLocalization()->DumpLatestMessage();
  AdapterManager::GetChassis()->DumpLatestMessage();
  AdapterManager::GetRoutingResponse()->DumpLatestMessage();
  if (FLAGS_enable_prediction) {
    AdapterManager::GetPrediction()->DumpLatestMessage();
  }
}

}  // namespace util
}  // namespace planning
}  // namespace apollo
