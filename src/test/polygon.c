/*=============================================================================
| Copyright 2012 Matthew D. Steele <mdsteele@alum.mit.edu>                    |
|                                                                             |
| This file is part of Azimuth.                                               |
|                                                                             |
| Azimuth is free software: you can redistribute it and/or modify it under    |
| the terms of the GNU General Public License as published by the Free        |
| Software Foundation, either version 3 of the License, or (at your option)   |
| any later version.                                                          |
|                                                                             |
| Azimuth is distributed in the hope that it will be useful, but WITHOUT      |
| ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       |
| FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for   |
| more details.                                                               |
|                                                                             |
| You should have received a copy of the GNU General Public License along     |
| with Azimuth.  If not, see <http://www.gnu.org/licenses/>.                  |
=============================================================================*/

#include <math.h>
#include <stdlib.h> // for NULL

#include "azimuth/util/polygon.h"
#include "azimuth/util/vector.h"
#include "test/test.h"

/*===========================================================================*/

static az_vector_t triangle_vertices[3] = {{-3, -3}, {2, 0}, {1, 4}};

static az_vector_t concave_hexagon_vertices[6] = {
  {2, -3}, {2, 2}, {-2, 3}, {-2, -3}, {0, -1}, {1, -1}
};

static az_vector_t square_vertices[5] =
  {{1, 1}, {0, 1}, {-1, 1}, {-1, -1}, {1, -1}};

/*===========================================================================*/

void test_polygon_contains(void) {
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);
  // Simple case: a convex polygon.
  EXPECT_TRUE(az_polygon_contains(triangle, (az_vector_t){0, 1}));
  EXPECT_TRUE(az_polygon_contains(triangle, AZ_VZERO));
  EXPECT_TRUE(az_polygon_contains(triangle, (az_vector_t){1.5, 0.5}));
  EXPECT_FALSE(az_polygon_contains(triangle, (az_vector_t){-2, 3}));
  EXPECT_FALSE(az_polygon_contains(triangle, (az_vector_t){5, 1}));
  EXPECT_FALSE(az_polygon_contains(triangle, (az_vector_t){-5, 10}));

  // Trickier: a concave polygon.
  const az_polygon_t hexagon = AZ_INIT_POLYGON(concave_hexagon_vertices);
  EXPECT_TRUE(az_polygon_contains(hexagon, AZ_VZERO));
  EXPECT_TRUE(az_polygon_contains(hexagon, (az_vector_t){-1, 2.5}));
  EXPECT_TRUE(az_polygon_contains(hexagon, (az_vector_t){-1.5, -2}));
  EXPECT_FALSE(az_polygon_contains(hexagon, (az_vector_t){0, -2}));
  EXPECT_FALSE(az_polygon_contains(hexagon, (az_vector_t){-3, -2}));
  EXPECT_TRUE(az_polygon_contains(hexagon, (az_vector_t){-1, -1}));
  EXPECT_FALSE(az_polygon_contains(hexagon, (az_vector_t){-5, -1}));

  // Let do some tests with colinear edges:
  const az_polygon_t square = AZ_INIT_POLYGON(square_vertices);
  EXPECT_TRUE(az_polygon_contains(square, AZ_VZERO));
  EXPECT_TRUE(az_polygon_contains(square, (az_vector_t){0.5, 0.5}));
  EXPECT_FALSE(az_polygon_contains(square, (az_vector_t){-5, 1}));
  EXPECT_FALSE(az_polygon_contains(square, (az_vector_t){-5, -1}));
}

void test_convex_polygon_contains(void) {
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);
  EXPECT_TRUE(az_convex_polygon_contains(triangle, (az_vector_t){0, 1}));
  EXPECT_TRUE(az_convex_polygon_contains(triangle, AZ_VZERO));
  EXPECT_TRUE(az_convex_polygon_contains(triangle, (az_vector_t){1.5, 0.5}));
  EXPECT_FALSE(az_convex_polygon_contains(triangle, (az_vector_t){-2, 3}));
  EXPECT_FALSE(az_convex_polygon_contains(triangle, (az_vector_t){5, 1}));
  EXPECT_FALSE(az_convex_polygon_contains(triangle, (az_vector_t){-5, 10}));

  // Let do some tests with colinear edges:
  const az_polygon_t square = AZ_INIT_POLYGON(square_vertices);
  EXPECT_TRUE(az_convex_polygon_contains(square, AZ_VZERO));
  EXPECT_TRUE(az_convex_polygon_contains(square, (az_vector_t){0.5, 0.5}));
  EXPECT_FALSE(az_convex_polygon_contains(square, (az_vector_t){-5, 1}));
  EXPECT_FALSE(az_convex_polygon_contains(square, (az_vector_t){-5, -1}));
}

/*===========================================================================*/

void test_ray_hits_polygon(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t intersect = nix, normal = nix;
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);

  // Check az_ray_hits_polygon works with NULLs for point_out and normal_out:
  EXPECT_TRUE(az_ray_hits_polygon(
      triangle, (az_vector_t){2, 4}, (az_vector_t){-1, -4}, NULL, NULL));

  // Check case where ray hits an edge:
  intersect = normal = nix;
  EXPECT_TRUE(az_ray_hits_polygon(
      triangle, (az_vector_t){2, 4}, (az_vector_t){-1, -4},
      &intersect, &normal));
  EXPECT_VAPPROX(((az_vector_t){1.5, 2}), intersect);
  EXPECT_VAPPROX(az_vunit((az_vector_t){4, 1}), az_vunit(normal));

  // Check case where ray hits several edges:
  intersect = normal = nix;
  EXPECT_TRUE(az_ray_hits_polygon(
      triangle, (az_vector_t){2, 4}, (az_vector_t){-5, -20},
      &intersect, &normal));
  EXPECT_VAPPROX(((az_vector_t){1.5, 2}), intersect);
  EXPECT_VAPPROX(az_vunit((az_vector_t){4, 1}), az_vunit(normal));

  // Check case where ray is entirely inside polygon (should hit at the start
  // point, with the normal being -delta):
  intersect = normal = nix;
  EXPECT_TRUE(az_ray_hits_polygon(
      triangle, (az_vector_t){0.5, 0}, (az_vector_t){1, 0},
      &intersect, &normal));
  EXPECT_VAPPROX(((az_vector_t){0.5, 0}), intersect);
  EXPECT_VAPPROX(((az_vector_t){-1, 0}), az_vunit(normal));

  // Check case where ray misses (by stopping short of polygon):
  intersect = normal = nix;
  EXPECT_FALSE(az_ray_hits_polygon(
      triangle, (az_vector_t){-5, 0}, (az_vector_t){1, 0},
      &intersect, &normal));
  EXPECT_VAPPROX(nix, intersect);
  EXPECT_VAPPROX(nix, normal);

  // Check case where ray misses (by missing completely):
  intersect = normal = nix;
  EXPECT_FALSE(az_ray_hits_polygon(
      triangle, (az_vector_t){5, 4}, (az_vector_t){1, -5},
      &intersect, &normal));
  EXPECT_VAPPROX(nix, intersect);
  EXPECT_VAPPROX(nix, normal);
}

void test_ray_hits_polygon_trans(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t intersect = nix, normal = nix;
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);

  intersect = normal = nix;
  EXPECT_TRUE(az_ray_hits_polygon_trans(
      triangle, (az_vector_t){3, 0.059714999709336247}, 1.3258176636680323,
      (az_vector_t){3, 5}, (az_vector_t){0, -5}, &intersect, &normal));
  EXPECT_VAPPROX(((az_vector_t){3, 2}), intersect);
  EXPECT_VAPPROX(((az_vector_t){0, 1}), az_vunit(normal));
}

/*===========================================================================*/

void test_circle_hits_point(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix;

  // Check az_circle_hits_point works with NULLs for pos_out and impact_out:
  EXPECT_TRUE(az_circle_hits_point(
      (az_vector_t){1, 1}, 2.0, (az_vector_t){-6, 1}, (az_vector_t){10, 0},
      NULL, NULL));

  // Check case where circle hits point dead-on.
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_point(
      (az_vector_t){1, 1}, 2.0, (az_vector_t){-6, 1}, (az_vector_t){10, 0},
      &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){-1, 1}), pos);
  EXPECT_VAPPROX(((az_vector_t){1, 1}), impact);

  // Check case where point starts inside circle.
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_point(
      (az_vector_t){-5, 7}, 2.0, (az_vector_t){-6, 6}, (az_vector_t){9, -15},
      &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){-6, 6}), pos);
  EXPECT_VAPPROX(((az_vector_t){-5, 7}), impact);

  // Check case where circle misses point.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_point(
      (az_vector_t){1, 1}, 2.0, (az_vector_t){-6, 1}, (az_vector_t){10, 10},
      &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);

  // Check case where circle stops short of point.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_point(
      (az_vector_t){1, 1}, 2.0, (az_vector_t){-6, 1}, (az_vector_t){3, 0},
      &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);
}

void test_circle_hits_line(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix;

  // Check az_circle_hits_line works with NULLs for pos_out and impact_out:
  EXPECT_TRUE(az_circle_hits_line(
      (az_vector_t){1, 1}, (az_vector_t){2, 1}, 2.0, (az_vector_t){15, -5},
      (az_vector_t){0, 10}, NULL, NULL));

  // Check case where circle hits line dead-on.
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_line(
      (az_vector_t){1, 1}, (az_vector_t){2, 1}, 2.0, (az_vector_t){15, -5},
      (az_vector_t){0, 10}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){15, -1}), pos);
  EXPECT_VAPPROX(((az_vector_t){15, 1}), impact);

  // Check case where circle is already intersecting line.
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_line(
      (az_vector_t){1, 1}, (az_vector_t){2, 1}, 2.0, (az_vector_t){12, 2},
      (az_vector_t){0, 10}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){12, 2}), pos);
  EXPECT_VAPPROX(((az_vector_t){12, 1}), impact);

  // Check case where circle goes wrong direction.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_line(
      (az_vector_t){1, 1}, (az_vector_t){2, 1}, 2.0, (az_vector_t){15, -5},
      (az_vector_t){0, -10}, &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);

  // Check case where circle stops short of line.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_line(
      (az_vector_t){1, 1}, (az_vector_t){2, 1}, 2.0, (az_vector_t){15, -5},
      (az_vector_t){0, 3}, &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);
}

void test_circle_hits_line_segment(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix;

  // Check az_circle_hits_line works with NULLs for pos_out and impact_out:
  EXPECT_TRUE(az_circle_hits_line_segment(
      (az_vector_t){5.5, 1}, (az_vector_t){6.5, 1}, 2.0, (az_vector_t){6, -5},
      (az_vector_t){0, 10}, NULL, NULL));

  // Check case where circle hits line segment dead-on.
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_line_segment(
      (az_vector_t){5.5, 1}, (az_vector_t){6.5, 1}, 2.0, (az_vector_t){6, -5},
      (az_vector_t){0, 10}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){6, -1}), pos);
  EXPECT_VAPPROX(((az_vector_t){6, 1}), impact);

  // Check case where circle would hit infinite line, but misses line segment.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_line_segment(
      (az_vector_t){5.5, 1}, (az_vector_t){6.5, 1}, 2.0, (az_vector_t){16, -5},
      (az_vector_t){0, 10}, &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);

  // Check case where circle would hit infinite line, but misses line segment
  // on the other side.
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_line_segment(
      (az_vector_t){5.5, 1}, (az_vector_t){6.5, 1}, 2.0, (az_vector_t){0, -5},
      (az_vector_t){0, 10}, &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);
}

void test_circle_hits_polygon(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix;
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);
  const az_polygon_t square = AZ_INIT_POLYGON(square_vertices);

  // Check az_circle_hits_polygon works with NULLs for pos_out and impact_out:
  EXPECT_TRUE(az_circle_hits_polygon(
      triangle, 2.0, (az_vector_t){3.940285000290664, 5.4850712500726659},
      (az_vector_t){-4, -10}, NULL, NULL));

  // Check case where circle hits an edge of the triangle:
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_polygon(
      triangle, 2.0, (az_vector_t){5.4402850002906638, 7.4850712500726662},
      (az_vector_t){-4, -10}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){3.4402850002906638, 2.4850712500726662}), pos);
  EXPECT_VAPPROX(((az_vector_t){1.5, 2}), impact);

  // Check case where circle hits a corner of the triangle:
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_polygon(
      triangle, 2.0, (az_vector_t){4, 7}, (az_vector_t){-10, -10},
      &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){2.4142135623730949, 5.4142135623730949}), pos);
  EXPECT_VAPPROX(((az_vector_t){1, 4}), impact);

  // Check case where circle hits an edge of the square:
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_polygon(
      square, 0.3, (az_vector_t){-5, 0}, (az_vector_t){20, 0}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){-1.3, 0}), pos);
  EXPECT_VAPPROX(((az_vector_t){-1, 0}), impact);

  // Check case where circle hits a corner of the square:
  pos = impact = nix;
  EXPECT_TRUE(az_circle_hits_polygon(
      square, sqrt(2), (az_vector_t){-5, 2}, (az_vector_t){20, 0},
      &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){-2, 2}), pos);
  EXPECT_VAPPROX(((az_vector_t){-1, 1}), impact);

  // Check case where circle misses the square:
  pos = impact = nix;
  EXPECT_FALSE(az_circle_hits_polygon(
      square, 0.2, (az_vector_t){-5, 1.21}, (az_vector_t){20, 0},
      &pos, &impact));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);
}

void test_circle_hits_polygon_trans(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix;
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);

  EXPECT_TRUE(az_circle_hits_polygon_trans(
      triangle, (az_vector_t){3, 0.059714999709336247}, 1.3258176636680323,
      2.0, (az_vector_t){3, 5}, (az_vector_t){0, -5}, &pos, &impact));
  EXPECT_VAPPROX(((az_vector_t){3, 4}), pos);
  EXPECT_VAPPROX(((az_vector_t){3, 2}), impact);
}

/*===========================================================================*/

void test_polygons_collide(void) {
  const az_vector_t nix = {99999, 99999};
  az_vector_t pos = nix, impact = nix, normal = nix;
  const az_polygon_t triangle = AZ_INIT_POLYGON(triangle_vertices);
  const az_polygon_t square = AZ_INIT_POLYGON(square_vertices);

  // Check az_polygons_collide works with NULLs for out args:
  pos = impact = normal = nix;
  EXPECT_TRUE(az_polygons_collide(
      square, AZ_VZERO, 0, triangle, (az_vector_t){0, -5}, AZ_HALF_PI,
      (az_vector_t){0, 10}, NULL, NULL, NULL));

  // Check case where corner of moving polygon hits edge of stationary polygon:
  pos = impact = normal = nix;
  EXPECT_TRUE(az_polygons_collide(
      square, AZ_VZERO, 0, triangle, (az_vector_t){0, -5}, AZ_HALF_PI,
      (az_vector_t){0, 10}, &pos, &impact, &normal));
  EXPECT_VAPPROX(((az_vector_t){0, -3}), pos);
  EXPECT_VAPPROX(((az_vector_t){0, -1}), impact);
  EXPECT_VAPPROX(((az_vector_t){0, -1}), az_vunit(normal));

  // Check case where corner of stationary polygon hits edge of moving polygon:
  pos = impact = normal = nix;
  EXPECT_TRUE(az_polygons_collide(
      triangle, AZ_VZERO, 0, square, (az_vector_t){-5, -3}, AZ_PI,
      (az_vector_t){10, 0}, &pos, &impact, &normal));
  EXPECT_VAPPROX(((az_vector_t){-4, -3}), pos);
  EXPECT_VAPPROX(((az_vector_t){-3, -3}), impact);
  EXPECT_VAPPROX(((az_vector_t){-1, 0}), az_vunit(normal));

  // Check another case where corner of stationary polygon hits edge of moving
  // polygon:
  pos = impact = normal = nix;
  EXPECT_TRUE(az_polygons_collide(
      triangle, AZ_VZERO, 0, square, (az_vector_t){-6, -6}, AZ_PI/4,
      (az_vector_t){10, 10}, &pos, &impact, &normal));
  EXPECT_VAPPROX(((az_vector_t){-3.707106781186548, -3.707106781186548}), pos);
  EXPECT_VAPPROX(((az_vector_t){-3, -3}), impact);
  EXPECT_VAPPROX(az_vunit((az_vector_t){-1, -1}), az_vunit(normal));

  // Check case where polygons don't collide:
  pos = impact = normal = nix;
  EXPECT_FALSE(az_polygons_collide(
      square, AZ_VZERO, 0, triangle, (az_vector_t){5, -3}, 1.3258176636680323,
      (az_vector_t){-10, 0}, &pos, &impact, &normal));
  EXPECT_VAPPROX(nix, pos);
  EXPECT_VAPPROX(nix, impact);
  EXPECT_VAPPROX(nix, normal);

  // Check case where stationary polygon is not at origin:
  pos = impact = normal = nix;
  EXPECT_TRUE(az_polygons_collide(
      triangle, (az_vector_t){5, -2}, -AZ_HALF_PI,
      square, (az_vector_t){-1, 4}, AZ_PI / 4,
      (az_vector_t){10, -10}, &pos, &impact, &normal));
  EXPECT_VAPPROX(((az_vector_t){1.2928932188134525, 1.7071067811865475}), pos);
  EXPECT_VAPPROX(((az_vector_t){2, 1}), impact);
  EXPECT_VAPPROX(az_vunit((az_vector_t){-1, 1}), az_vunit(normal));
}

/*===========================================================================*/
