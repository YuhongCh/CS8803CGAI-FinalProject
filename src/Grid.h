#pragma once

#include "Definition.h"
#include "Array.h"

class Grid {
public:
	Grid(const Vector3& minCoord, const Vector3& maxCoord, const Vector3i& dimension)
		: m_minCoord(minCoord), m_maxCoord(maxCoord), m_dimension(dimension) {
		m_gridData = Array3<Scalar>(dimension.x(), dimension.y(), dimension.z());
	}

	inline bool IsInGrid(const Vector3& pos) const {
		return m_minCoord.x() <= pos.x() && m_minCoord.y() <= pos.y() && m_minCoord.z() <= pos.z() &&
			pos.x() <= m_maxCoord.x() && pos.y() <= m_maxCoord.y() && pos.z() <= m_maxCoord.z();
	}

	inline bool IsInGrid(const Scalar& x, const Scalar& y, const Scalar& z) const {
		return m_minCoord.x() <= x && m_minCoord.y() <= y && m_minCoord.z() <= z &&
			x <= m_maxCoord.x() && y <= m_maxCoord.y() && z <= m_maxCoord.z();
	}

	inline bool IsValidIndex(const Vector3i& index) const {
		return 0 <= index.x() && 0 <= index.y() && 0 <= index.z() &&
			index.x() < m_dimension.x() && index.y() < m_dimension.y() && index.z() < m_dimension.z();
	}

	inline bool IsValidIndex(Integer xi, Integer yi, Integer zi) const {
		return 0 <= xi && 0 <= yi && 0 <= zi && xi < m_dimension.x() && yi < m_dimension.y() && zi < m_dimension.z();
	}

	inline const Vector3& GetMinCoord() const { return m_minCoord; }
	inline const Vector3& GetMaxCoord() const { return m_maxCoord; }

	inline const Vector3i& GetDimension() const { return m_dimension; }

	inline Vector3 GetGridSize() const {
		return (m_maxCoord - m_minCoord).cwiseQuotient(m_dimension);
	}

	inline Scalar& GetGridData(Integer xi, Integer yi, Integer zi) { return m_gridData(xi, yi, zi); }
	inline Scalar& GetGridData(const Vector3i& index) { return m_gridData(index); }
	inline void SetGridData(Integer xi, Integer yi, Integer zi, const Scalar& value) { m_gridData(xi, yi, zi) = value; }
	inline void SetGridData(const Vector3i& index, const Scalar& value) { m_gridData(index) = value; }

private:
	Vector3 m_minCoord;
	Vector3 m_maxCoord;
	Vector3i m_dimension;
	Array3<Scalar> m_gridData;

};