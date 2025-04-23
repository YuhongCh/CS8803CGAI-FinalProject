#pragma once
#include "../Definition.h"

class Mesh {

public:
	Mesh() {}

public:
	inline void SetVertices(const std::vector<Vector3f>& vertices) { m_Vertices = std::move_if_noexcept(vertices); }
	inline void SetTriangles(const std::vector<Vector3i>& triangles) { m_Triangles = std::move_if_noexcept(triangles); }
	inline void SetUVs(const std::vector<Vector2f>& uvs) { m_UVs = std::move_if_noexcept(uvs); }
	inline void SetNormals(const std::vector<Vector3f>& normals) { m_Normals = std::move_if_noexcept(normals); }
	inline void SetColors(const std::vector<Vector3f>& colors) { m_Colors = std::move_if_noexcept(colors); }

	inline void SetVertex(const Vector3f& vertex, Integer index) { m_Vertices[index] = vertex; }
	inline void SetTriangle(const Vector3i& triIndices, Integer index) { m_Triangles[index] = triIndices; }
	inline void SetUV(const Vector2f& uvs, Integer index) { m_UVs[index] = uvs; }
	inline void SetNormal(const Vector3f& normal, Integer index) { m_Normals[index] = normal; }
	inline void SetColor(const Vector3f& color, Integer index) { m_Colors[index] = color; }

	inline const std::vector<Vector3f>& GetVertices() const { return m_Vertices; }
	inline const std::vector<Vector3i>& GetTriangles() const { return m_Triangles; }
	inline const std::vector<Vector2f>& GetUVs() const { return m_UVs; }
	inline const std::vector<Vector3f>& GetNormals() const { return m_Normals; }
	inline const std::vector<Vector3f>& GetColors() const { return m_Colors; }

	inline const Vector3f& GetVertex(Integer index) const { return m_Vertices[index]; }
	inline const Vector3i& GetTriangle(Integer index) const { return m_Triangles[index]; }
	inline const Vector2f& GetUV(Integer index) const { return m_UVs[index]; }
	inline const Vector3f& GetNormal(Integer index) const { return m_Normals[index]; }
	inline const Vector3f& GetColor(Integer index) const { return m_Colors[index]; }

	/// <summary>
	/// Read and Write mesh file, currently only ply file is supported
	/// </summary>
	/// <param name="filename"></param>
	void Read(const std::string& filename);
	void Write(const std::string& filename, bool useBinary = true) const;

	/// <summary>
	/// Find the number of vertices of the mesh
	/// </summary>
	/// <returns>number of vertices</returns>
	inline Integer Size() const { return m_Vertices.size(); }

	/// <summary>
	/// Recalculate the normal vector of the vertices based on the vertices' position
	/// </summary>
	void ReCalculateNormals();

	/// <summary>
	/// Rough Validity check against content of the Mesh. It checks for
	/// 1) if size of different mesh element agrees with each other
	/// 2) if triangle indices get out of range
	/// </summary>
	/// <returns>return true if validity test passes, return false otherwise</returns>
	bool IsValid() const;

private:
	std::vector<Vector3f> m_Vertices;
	std::vector<Vector3i> m_Triangles;
	std::vector<Vector2f> m_UVs;
	std::vector<Vector3f> m_Normals;
	std::vector<Vector3f> m_Colors;
};