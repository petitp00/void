#include "stdafx.h"
#include "Collisions.h"

#include <algorithm>

namespace Collision {

	Box::Box(Cube cube) : position(cube.position), size(cube.size)
	{}

	HalfBox::HalfBox(HalfCube hcube) : position(hcube.position), size(hcube.size), rotation(hcube.rotation)
	{
		float angle = 0.f;
		if (rotation.x == -1) {
			angle = glm::radians(180.f);
		}
		else if (rotation.y == -1) {
			angle = glm::radians(90.f);
		}
		else if (rotation.y == 1) {
			angle = glm::radians(270.f);
		}

		rotation_matrix = glm::rotate(glm::mat4(), angle, glm::vec3(0, 1, 0));
	}

	Result Collision::Get(Box b1, Box b2)
	{
		Result result;
		result.collision = false;

		glm::vec3 b1p = b1.position;
		glm::vec3 b1s = b1.size * 0.5f; // because the origin of the box is at the center
		glm::vec3 b2p = b2.position;
		glm::vec3 b2s = b2.size * 0.5f;

		// x axis
		float dxp = b1p.x + b1s.x - (b2p.x - b2s.x);
		if (dxp <= 0) return result; // no overlap
		float dxm = b1p.x - b1s.x - (b2p.x + b2s.x);
		if (dxm >= 0) return result; // no overlap
		float mtvx = 1000000000;
		if		(b1p.x < b2p.x) mtvx = -dxp;
		else if (b1p.x > b2p.x)	mtvx = -dxm;

		// z axis
		float dzp = b1p.z + b1s.z - (b2p.z - b2s.z);
		if (dzp <= 0) return result; // no overlap
		float dzm = b1p.z - b1s.z - (b2p.z + b2s.z);
		if (dzm >= 0) return result; // no overlap
		float mtvz = 1000000000;
		if		(b1p.z < b2p.z) mtvz = -dzp;
		else if (b1p.z > b2p.z)	mtvz = -dzm;

		// y axis
		float dyp = b1p.y + b1s.y - (b2p.y - b2s.y);
		if (dyp <= 0) return result; // no overlap
		float dym = b1p.y - b1s.y - (b2p.y + b2s.y);
		if (dym >= 0) return result; // no overlap
		float mtvy = 1000000000; // set really high in case b1p.y == b2p.y (we don't want mtv to be 0)
		if		(b1p.y < b2p.y) mtvy = -dyp;
		else if (b1p.y > b2p.y)	mtvy = -dym;

		result.collision = true;
		glm::vec3 mtv;
		float min_mtv_val = 10000;
		if (abs(mtvx) < abs(min_mtv_val)) {
			min_mtv_val = mtvx;
			mtv = glm::vec3(mtvx, 0, 0);
		}
		if (abs(mtvy) < abs(min_mtv_val)) {
			min_mtv_val = mtvy;
			mtv = glm::vec3(0, mtvy, 0);
		}
		if (abs(mtvz) < abs(min_mtv_val)) {
			mtv = glm::vec3(0, 0, mtvz);
		}
		result.mtv = mtv;
		return result;
	}

	Result Get(Box b1, HalfBox b2)
	{
		Result result;
		result.collision = false;

		glm::vec3 b1p = b1.position;
		glm::vec3 b1s = b1.size * 0.5f; // because the origin of the box is at the center

		glm::vec3 b2p = b2.position;
		glm::vec3 b2s = b2.rotation_matrix * glm::vec4(b2.size, 1.0f) * 0.5f;
		b2s.x = abs(b2s.x);
		b2s.z = abs(b2s.z);

		//std::cout << b2s << '\n';

		// x axis
		float dxp = b1p.x + b1s.x - (b2p.x - b2s.x);
		if (dxp <= 0) return result; // no overlap
		float dxm = b1p.x - b1s.x - (b2p.x + b2s.x);
		if (dxm >= 0) return result; // no overlap
		float mtvx = 1000000000;
		if		(b1p.x < b2p.x) mtvx = -dxp;
		else if (b1p.x > b2p.x)	mtvx = -dxm;

		// z axis
		float dzp = b1p.z + b1s.z - (b2p.z - b2s.z);
		if (dzp <= 0) return result; // no overlap
		float dzm = b1p.z - b1s.z - (b2p.z + b2s.z);
		if (dzm >= 0) return result; // no overlap
		float mtvz = 1000000000;
		if		(b1p.z < b2p.z) mtvz = -dzp;
		else if (b1p.z > b2p.z)	mtvz = -dzm;

		// y axis
		float dyp = b1p.y + b1s.y - (b2p.y - b2s.y);
		if (dyp <= 0) return result; // no overlap
		float dym = b1p.y - b1s.y - (b2p.y + b2s.y);
		if (dym >= 0) return result; // no overlap
		float mtvy = 1000000000; // set really high in case b1p.y == b2p.y (we don't want mtv to be 0)
		if		(b1p.y < b2p.y) mtvy = -dyp;
		else if (b1p.y > b2p.y)	mtvy = -dym;

		glm::vec3 mtv;
		float min_mtv_val = 10000;
		if (abs(mtvx) < abs(min_mtv_val)) {
			min_mtv_val = mtvx;
			mtv = glm::vec3(mtvx, 0, 0);
		}
		if (abs(mtvy) < abs(min_mtv_val)) {
			min_mtv_val = mtvy;
			mtv = glm::vec3(0, mtvy, 0);
		}
		if (abs(mtvz) < abs(min_mtv_val)) {
			mtv = glm::vec3(0, 0, mtvz);
		}

		// try projecting vertices on plane perpendicular to slope
		glm::vec3 slope = glm::normalize(glm::vec3(b2.size.x * b2.rotation.x, b2.size.y, b2.size.x * b2.rotation.y));
		glm::mat4 r = glm::rotate(glm::mat4(), glm::radians(90.f * b2.rotation.x - 90.f * b2.rotation.y), glm::vec3(abs(b2.rotation.y), 0, abs(b2.rotation.x)));
		glm::vec3 perp = glm::normalize(r * glm::vec4(slope, 1.f));

		float mina =  100000;
		float maxa = -100000;
		float minb =  100000;
		float maxb = -100000;

		std::vector<glm::vec3> player_points = {
			b1p + glm::vec3(-b1s.x, -b1s.y, -b1s.z),
			b1p + glm::vec3(-b1s.x, -b1s.y,  b1s.z),
			b1p + glm::vec3(-b1s.x,  b1s.y, -b1s.z),
			b1p + glm::vec3(-b1s.x,  b1s.y,  b1s.z),
			b1p + glm::vec3( b1s.x, -b1s.y, -b1s.z),
			b1p + glm::vec3( b1s.x, -b1s.y,  b1s.z),
			b1p + glm::vec3( b1s.x,  b1s.y, -b1s.z),
			b1p + glm::vec3( b1s.x,  b1s.y,  b1s.z)
		};

		for (auto p : player_points) {
			float proj = glm::dot(p, perp);
			if (proj <= mina) { mina = proj; }
			if (proj >= maxa) { maxa = proj; }
		}

		b2s = b2.size * 0.5f;

		std::vector<glm::vec3> hbox_points = {
			b2p + glm::vec3(
				abs(b2.rotation.x) * b2s.x + abs(b2.rotation.y) * b2s.z,
				-b2s.y,
				abs(b2.rotation.x) * b2s.z + abs(b2.rotation.y) * b2s.x
			),
			b2p + glm::vec3(
				abs(b2.rotation.x) * b2s.x + abs(b2.rotation.y) * b2s.z,
				-b2s.y,
				-(abs(b2.rotation.x) * b2s.z + abs(b2.rotation.y) * b2s.x)
			),
			b2p + glm::vec3(
				-(abs(b2.rotation.x) * b2s.x + abs(b2.rotation.y) * b2s.z),
				-b2s.y,
				abs(b2.rotation.x) * b2s.z + abs(b2.rotation.y) * b2s.x
			),
			b2p + glm::vec3(
				-(abs(b2.rotation.x) * b2s.x + abs(b2.rotation.y) * b2s.z),
				-b2s.y,
				-(abs(b2.rotation.x) * b2s.z + abs(b2.rotation.y) * b2s.x)
			),
			b2p + glm::vec3(
				b2.rotation.x * b2s.x + abs(b2.rotation.y) * b2s.z,
				b2s.y,
				b2.rotation.y * b2s.x + abs(b2.rotation.x) * b2s.z
			),
			b2p + glm::vec3(
				b2.rotation.x * b2s.x - abs(b2.rotation.y) * b2s.z,
				b2s.y,
				b2.rotation.y * b2s.x - abs(b2.rotation.x) * b2s.z
			)
		};

		for (auto p : hbox_points) {
			float proj = glm::dot(p, perp);
			if (proj <= minb) { minb = proj; }
			if (proj >= maxb) { maxb = proj; }
		}

		if (maxa - minb >= 0 && maxb - mina >= 0) {
			auto mtv_sat = perp * std::min(maxb - mina, maxa - minb) / glm::dot(perp, perp);
			if (b2.size.x >= b2.size.y) {
				mtv_sat = glm::vec3(0, mtv_sat.y, 0);
			}
			if (glm::length(mtv_sat) < glm::length(mtv)) {
				mtv = mtv_sat;
			}
		}
		else {
			return result;
		}


		result.collision = true;

		result.mtv = mtv;
		return result;
	}
}
