#include "Assets.hpp"
#include "Crash.hpp"
#include "Logs.hpp"
#include "Files.hpp"
																														// TODO doing this cpu side is prob bad
#define POST_PROCESS aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_PreTransformVertices
// aiProcessPreset_TargetRealtime_MaxQuality

void process_mesh(const aiMesh *mesh, GameObject *obj) {
	GLuint index_offset = obj->verts.size();

	// const GLuint material_id = mesh->mMaterialIndex;

	// go over all vertices of the mesh (they are not necessarily in any order)
	if (mesh->HasNormals() && mesh->HasTextureCoords(0)) { // 0 here cursed too
		for (unsigned int v = 0; v < mesh->mNumVertices; v++) {
			const aiVector3D &pos = mesh->mVertices[v];
			const aiVector3D &normal = mesh->mNormals[v];
			const aiVector3D &texcoords = mesh->mTextureCoords[0][v]; // [0] cursed, a vertex can contain up to 8 different texture coordinates
			obj->verts.emplace_back(glm::vec3(pos.x, pos.y, pos.z), glm::vec3(normal.x, normal.y, normal.z), glm::vec2(texcoords.x, texcoords.y));
		}
	} else {
		Log::log(LOG_TYPE::ERR, std::string(__PRETTY_FUNCTION__), "Mesh does not have normals, not loading anything for now");
	}

	// go over all faces of the mesh, giving me the indices
	// now I can actually get the vertices in the order they are modeled by using their index, so I can make the phys mesh
	for(unsigned int f = 0; f < mesh->mNumFaces; f++)
	{
		const aiFace face = mesh->mFaces[f];
		JPH::Vec3 phys_verts[3];
		// I assume faces are all triangles since I use aiProcess_Triangulate, TODO make an assert or something
		CRASH_IF(face.mNumIndices != 3, "Face is not a triangle");

		for(unsigned int j = 0; j < face.mNumIndices; j++) {
			obj->indices.emplace_back(face.mIndices[j] + index_offset);
			const aiVector3D &pos = mesh->mVertices[face.mIndices[j]];
			phys_verts[j] = JPH::Vec3(pos.x, pos.y, pos.z);
		}
		// obj->phys_triangles.emplace_back(phys_verts[0], phys_verts[1], phys_verts[2]);
	}
}

void recursive_add_verts(const aiScene* scene, const aiNode *node, GameObject *obj) {
	// since N meshes, need to offset index

	// go over all meshes of the node
	for (unsigned int m = 0; m < node->mNumMeshes; m++) {
		const aiMesh *mesh = scene->mMeshes[node->mMeshes[m]];

		process_mesh(mesh, obj);
	}

	// repeat for all children
	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		recursive_add_verts(scene,  node->mChildren[n], obj);
	}
}

void Assets::load(const std::string &name, const std::string &hitbox, CustomVec<GameObject> &objs) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

	 // If the import failed, report it
	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	// importer destructor will clean everything up!!!!!!

	// scene has nodes
	// nodes have num meshes, and an array of indices to meshes
	// these indices are for an array in the scene itself

	// need to iterate over the nodes recursively and add up the vertices that make up their meshes
	// TODO test this vs iterating over things already in scene

	objs.emplace_back(1); // getNextPowerOfTwo(mesh->mNumVertices));

	aiNode *node = scene->mRootNode;

	recursive_add_verts(scene, node, objs.getBackPointer());

	FileHandler hitbox_file = FileHandler(ASSETS_FOLDER + hitbox);
	(objs.getBackPointer())->phys_shape = Phys::createShapeFromJson(hitbox_file.readjson());
}

void Assets::load(const std::string &name, CustomVec<GameObject> &objs) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

	 // If the import failed, report it
	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	// importer destructor will clean everything up!!!!!!

	// scene has nodes
	// nodes have num meshes, and an array of indices to meshes
	// these indices are for an array in the scene itself

	// need to iterate over the nodes recursively and add up the vertices that make up their meshes
	// TODO test this vs iterating over things already in scene

	objs.emplace_back(1); // getNextPowerOfTwo(mesh->mNumVertices));
	GameObject *obj = objs.getBackPointer();

	aiNode *node = scene->mRootNode;

	recursive_add_verts(scene, node, obj);

	obj->phys_shape = Phys::createConvexHull(obj->verts, obj->indices);
}

void Assets::loadMarchingCubes(const std::string &name, CustomVec<MarchingCubesObject> &objs, uint32_t len_x, uint32_t len_y, uint32_t len_z) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	objs.emplace_back(len_x, len_y, len_z);
	MarchingCubesObject *obj = objs.getBackPointer();
	aiNode *node = scene->mRootNode;
	GameObject gameobj = GameObject(1);

	// load the mesh for the model
	recursive_add_verts(scene, node, &gameobj);

	// turn the mesh into marching cubes
	EmbreeWrapper::marcheCubes(obj, gameobj.verts, gameobj.indices, len_x, len_y, len_z);

	// TODO free things here???
}


void recursive_dump_metadata(const aiNode *node) {
	aiMetadata *meta = node->mMetaData;

	printf("Name: %s\n", node->mName.C_Str());

	if (meta == nullptr) {
		printf("No metadata\n");
	} else {
		aiMetadataEntry value;
		printf("Metadata:\n");
		for (unsigned int i = 0; i < meta->mNumProperties; i++) {
			printf("%u - %s -", i, meta->mKeys[i].C_Str());

			value = meta->mValues[i];

			if (value.mData == nullptr) {
				printf("empty");
				continue;
			}

			switch (value.mType) {
				case aiMetadataType::AI_BOOL:
					printf("bool not implemented\n");
					break;
				case aiMetadataType::AI_INT32:
					printf("int32 not implemented\n");
					break;
				case aiMetadataType::AI_UINT64:
					printf("uint64 not implemented\n");
					break;
				case aiMetadataType::AI_FLOAT:
					printf("float not implemented\n");
					break;
				case aiMetadataType::AI_DOUBLE:
					printf("double not implemented\n");
					break;
				case aiMetadataType::AI_AISTRING:
					printf("string not implemented\n");
					break;
				case aiMetadataType::AI_AIVECTOR3D:
					printf("vec3 not implemented\n");
					break;
				case aiMetadataType::AI_AIMETADATA:
					printf("metadata(\?\?\?) not implemented\n");
					break;
				case aiMetadataType::AI_INT64:
					printf("int64 not implemented\n");
					break;
				case aiMetadataType::AI_UINT32:
					printf("uint32 not implemented\n");
					break;
				default:
					printf("Invalid type\n");
					break;
			}
		}
	}

	// repeat for all children
	for (unsigned int n = 0; n < node->mNumChildren; n++) {
		recursive_dump_metadata(node->mChildren[n]);
	}
}

void Assets::dumpMetadata(const std::string &name) {
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ASSETS_FOLDER + name, POST_PROCESS);

	 // If the import failed, report it
	CRASH_IF(scene == nullptr, std::string("Failed to import asset from ") + ASSETS_FOLDER + name);

	aiNode *node = scene->mRootNode;

	recursive_dump_metadata(node);
}
