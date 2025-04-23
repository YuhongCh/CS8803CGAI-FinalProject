#include "Mesh.h"
#include "../Utils/MathUtils.h"

#include <tinyply/tinyply.h>

bool Mesh::IsValid() const {
    Integer vertexSize = Size();

    bool cond1 = true;
    if (!m_UVs.empty()) cond1 &= vertexSize == m_UVs.size();
    if (!m_Normals.empty()) cond1 &= vertexSize == m_Normals.size();
    if (!m_Colors.empty()) cond1 &= vertexSize == m_Colors.size();
	if (!cond1) return false;

	bool cond2 = true;
	for (const Vector3i& tri : m_Triangles) {
		bool isLarger = tri.x() >= vertexSize || tri.y() >= vertexSize || tri.z() >= vertexSize;
		bool isSmaller = tri.x() < 0 || tri.y() < 0 || tri.z() < 0;
		if (isLarger || isSmaller) {
			cond2 = false;
			break;
		}
	}
	if (!cond2) return false;


	return true;
}

void Mesh::ReCalculateNormals() {
	Integer vertexSize = Size();
	std::vector<Integer> counter(vertexSize, 0);
	for (int index = 0; index < vertexSize; ++index) {
		m_Normals[index] = Vector3f::Zero();
	}

	for (const Vector3i& tri : m_Triangles) {
		Integer tri0 = tri.x(), tri1 = tri.y(), tri2 = tri.z();
		m_Normals[tri0] += Cross(m_Vertices[tri0], m_Vertices[tri1], m_Vertices[tri2]);
		m_Normals[tri1] += Cross(m_Vertices[tri1], m_Vertices[tri2], m_Vertices[tri0]);
		m_Normals[tri2] += Cross(m_Vertices[tri2], m_Vertices[tri0], m_Vertices[tri1]);
		++counter[tri0], ++counter[tri1], ++counter[tri2];
	}
	
	for (int index = 0; index < vertexSize; ++index) {
		m_Normals[index] /= counter[index];
	}
}

/// <summary>
/// This is used with tinyply to compare between custom data structure
/// </summary>
namespace tinyply {
    bool IsSame(Type plyType, const type_info& rhs) noexcept {
        switch (plyType) {
        case Type::INT8: return typeid(char) == rhs;
        case Type::UINT8: return typeid(unsigned char) == rhs;
        case Type::INT16: return typeid(short) == rhs;
        case Type::UINT16: return typeid(unsigned short) == rhs;
        case Type::INT32: return typeid(int) == rhs;
        case Type::UINT32: return typeid(unsigned int) == rhs;
        case Type::FLOAT32: return typeid(float) == rhs;
        case Type::FLOAT64: return typeid(double) == rhs;
        case Type::INVALID: return false;
        default: return false;
        }
    }


}

template<typename T, int S>
    requires CommonVectorType<T, S>
static std::vector<Vector<T, S>> ReadMeshProperty(std::shared_ptr<tinyply::PlyData> property) {
    if(property == nullptr) return std::vector<Vector<T, S>>();

    // Check for the valid type
    if constexpr (std::is_same<T, Scalar>::value) {
        CHECK((property->t == tinyply::Type::FLOAT32 || property->t == tinyply::Type::FLOAT64),
            "Property elements have unexpected data type");
    }
    else if constexpr (std::is_same<T, Integer>::value) {
        CHECK((property->t == tinyply::Type::INT32 || property->t == tinyply::Type::UINT32),
            "Property elements have unexpected data type");
    }
    else {
        CHECK(false, "Unexpected template type");
    }

    // With same data type, we can easily load the data
    if (property->count == 0) return std::vector<Vector<T, S>>();
    if (tinyply::IsSame(property->t, typeid(T))) {
        const size_t numBytes = property->buffer.size_bytes();
        std::vector<Vector<T, S>> props(property->count);
        std::memcpy(props.data(), property->buffer.get(), numBytes);
        return props;
    }

    std::vector<Vector<T, S>> props;
    if constexpr (std::is_same<T, float>::value) {
        std::vector<Vector<double, S>> buffer(property->count);
        std::memcpy(buffer.data(), property->buffer.get(), property->buffer.size_bytes());
        props.reserve(property->count);
        for (const auto& value : buffer) {
            props.push_back(value.cast<T>());
        }
    }
    else if constexpr (std::is_same<T, double>::value) {
        std::vector<Vector<float, S>> buffer(property->count);
        std::memcpy(buffer.data(), property->buffer.get(), property->buffer.size_bytes());
        props.reserve(property->count);
        for (const auto& value : buffer) {
            props.push_back(value.cast<T>());
        }
    }
    else if constexpr (std::is_same<T, Int32>::value) {
        std::vector<Vector<UInt32, S>> buffer(property->count);
        std::memcpy(buffer.data(), property->buffer.get(), property->buffer.size_bytes());
        props.reserve(property->count);
        for (const auto& value : buffer) {
            props.push_back(value.cast<T>());
        }
    }
    else if constexpr (std::is_same<T, UInt32>::value) {
        std::vector<Vector<Int32, S>> buffer(property->count);
        std::memcpy(buffer.data(), property->buffer.get(), property->buffer.size_bytes());
        props.reserve(property->count);
        for (const auto& value : buffer) {
            props.push_back(value.cast<T>());
        }
    }
    else {
        CHECK(false, "Unexpected template T data type");
    }
    return props;
}

/*
The ply  operations are implemented following example:
https://github.com/ddiakopoulos/tinyply/blob/master/source/example.cpp
*/

void Mesh::Read(const std::string& filePath) {
#ifdef _DEBUG
    std::cout << "MeshFilter start loading mesh data from " << filePath << "\n";
#endif
    std::unique_ptr<std::istream> file_stream;

    try {
        file_stream.reset(new std::ifstream(filePath, std::ios::binary));
        if (!file_stream || file_stream->fail()) {
            throw std::runtime_error("file_stream failed to open " + filePath);
        }
        file_stream->seekg(0, std::ios::beg);

        tinyply::PlyFile file;
        file.parse_header(*file_stream);
        std::shared_ptr<tinyply::PlyData> vertices, normals, colors, texcoords, faces;

        try { vertices = file.request_properties_from_element("vertex", { "x", "y", "z" }); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
        try { normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" }); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
        try { colors = file.request_properties_from_element("vertex", { "r", "g", "b", "a" }); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
        try { texcoords = file.request_properties_from_element("vertex", { "u", "v" }); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
        try { faces = file.request_properties_from_element("face", { "vertex_indices" }, 3); }
        catch (const std::exception& e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }
        file.read(*file_stream);

#ifdef _DEBUG
        if (vertices)   std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
        if (normals)    std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
        if (colors)     std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
        if (texcoords)  std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
        if (faces)      std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
#endif

        // mesh needs vertices and triangles at minimum
        std::vector<Vector3f> meshVertices = ReadMeshProperty<Scalar, 3>(vertices);
        std::vector<Vector3i> meshTriangles = ReadMeshProperty<Integer, 3>(faces);
        std::vector<Vector2f> meshUVs = ReadMeshProperty<Scalar, 2>(texcoords);
        std::vector<Vector3f> meshNormals = ReadMeshProperty<Scalar, 3>(normals);
        std::vector<Vector3f> meshColors = ReadMeshProperty<Scalar, 3>(colors);

        SetVertices(meshVertices);
        SetTriangles(meshTriangles);
        SetUVs(meshUVs);
        SetNormals(meshNormals);
        SetColors(meshColors);

        CHECK(IsValid(), "Failed to pass validity test on the mesh data");
    }
    catch (const std::exception& e) {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
    }
#ifdef _DEBUG
    std::cout << "MeshFilter complete loading mesh data from " << filePath << "\n";
#endif
}

void Mesh::Write(const std::string& filename, bool useBinary) const {
#ifdef _DEBUG
    std::cout << "MeshFilter start writing mesh data to " << filename << "\n";
#endif

    CHECK((filename.length() > 4 && filename.substr(filename.length() - 4) == ".ply"), "Invalid filename format");

    std::filebuf fbuffer;
    if (useBinary) fbuffer.open(filename, std::ios::out | std::ios::binary);
    else fbuffer.open(filename, std::ios::out);
    std::ostream outstream(&fbuffer);
    if (outstream.fail()) throw std::runtime_error("failed to open " + filename);

    tinyply::PlyFile plyFile;

    tinyply::Type plyFloatType = tinyply::Type::INVALID, plyIntType = tinyply::Type::INVALID;
    if constexpr (std::is_same<Scalar, float>::value) plyFloatType = tinyply::Type::FLOAT32;
    else if constexpr (std::is_same<Scalar, double>::value) plyFloatType = tinyply::Type::FLOAT64;
    if constexpr (std::is_same<Integer, Int32>::value) plyIntType = tinyply::Type::INT32;
    else if constexpr (std::is_same<Integer, UInt32>::value) plyIntType = tinyply::Type::UINT32;


    plyFile.add_properties_to_element("vertex", { "x", "y", "z" },
        plyFloatType, m_Vertices.size(), reinterpret_cast<const UInt8*>(m_Vertices.data()), tinyply::Type::INVALID, 0);
    plyFile.add_properties_to_element("face", { "vertex_indices" },
        plyIntType, m_Triangles.size(), reinterpret_cast<const UInt8*>(m_Triangles.data()), tinyply::Type::UINT8, 3);

    if (!m_Normals.empty()) {
        plyFile.add_properties_to_element("vertex", { "nx", "ny", "nz" },
            plyFloatType, m_Normals.size(), reinterpret_cast<const UInt8*>(m_Normals.data()), tinyply::Type::INVALID, 0);
    }
    if (!m_UVs.empty()) {
        plyFile.add_properties_to_element("vertex", { "u", "v" },
            plyFloatType, m_UVs.size(), reinterpret_cast<const UInt8*>(m_UVs.data()), tinyply::Type::INVALID, 0);
    }
    if (!m_Colors.empty()) {
        plyFile.add_properties_to_element("vertex", { "r", "g", "b", "a" },
            plyFloatType, m_Colors.size(), reinterpret_cast<const UInt8*>(m_Colors.data()), tinyply::Type::INVALID, 0);
    }

    plyFile.get_comments().push_back("generated by tinyply 2.3");
    plyFile.write(outstream, useBinary);

#ifdef _DEBUG
    std::cout << "MeshFilter complete writing mesh data to " << filename << "\n";
#endif
}

