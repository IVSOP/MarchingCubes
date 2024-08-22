#include "Archive.hpp"

// // reads size_t from file with the total size,
// // reads N bytes from file using that values as the number of bytes
// // places everything in this->data
// CustomArchive::CustomArchive(FileHandler &file)
// : data(1)
// {
// 	size_t len;
// 	file.read(&len, sizeof(size_t));
// 	data.reserve(len);
// 	void *buf = std::malloc(len);
// 	// TODO
// 	(void)file.read(buf, len);
// 	data.copy_bytes(buf, len);
// 	free(buf);
// }

template<>
void CustomArchive::serializeIntoBuffer<uint32_t>(const uint32_t &val) { // reference is cursed here but whatever
	data.reserve(sizeof(uint32_t));
	data.copy_bytes(&val, sizeof(uint32_t));
}

template<>
void CustomArchive::serializeIntoBuffer<size_t>(const size_t &val) { // reference is cursed here but whatever
	data.reserve(sizeof(size_t));
	data.copy_bytes(&val, sizeof(size_t));
}

template<>
void CustomArchive::serializeIntoBuffer<Component>(const Component &val) { // reference is cursed here but whatever
	data.reserve(sizeof(Component));
	data.copy_bytes(&val, sizeof(Component));
}


template<>
void CustomArchive::serializeIntoBuffer<glm::vec3>(const glm::vec3 &vec) {
	data.reserve(sizeof(glm::vec3));
	data.copy_bytes(&vec, sizeof(glm::vec3));
}

template<>
void CustomArchive::serializeIntoBuffer<GLfloat>(const GLfloat &val) {
	data.reserve(sizeof(GLfloat));
	data.copy_bytes(&val, sizeof(GLfloat));
}

template<>
void CustomArchive::serializeIntoBuffer<bool>(const bool &val) {
	data.reserve(sizeof(bool));
	data.copy_bytes(&val, sizeof(bool));
}

template<>
void CustomArchive::serializeIntoBuffer<Position>(const Position &pos) {
	Component component_id = Component::Position;
	serializeIntoBuffer<Component>(component_id);
	serializeIntoBuffer<glm::vec3>(pos.pos);
}

template<>
void CustomArchive::serializeIntoBuffer<Direction>(const Direction &dir) {
	Component component_id = Component::Direction;
	serializeIntoBuffer<Component>(component_id);
	serializeIntoBuffer<GLfloat>(dir.pitch);
	serializeIntoBuffer<GLfloat>(dir.yaw);
	serializeIntoBuffer<glm::vec3>(dir.front);
	serializeIntoBuffer<glm::vec3>(dir.worldup);
	serializeIntoBuffer<glm::vec3>(dir.up);
	serializeIntoBuffer<glm::vec3>(dir.right);
}

template<>
void CustomArchive::serializeIntoBuffer<Movement>(const Movement &mov) {
	Component component_id = Component::Movement;
	serializeIntoBuffer<Component>(component_id);
	serializeIntoBuffer<GLfloat>(mov.speed);
	serializeIntoBuffer<bool>(mov.speedup);
}

template<>
void CustomArchive::serializeIntoBuffer<Render>(const Render &render) {
	Component component_id = Component::Render;
	serializeIntoBuffer<Component>(component_id);
	serializeIntoBuffer<uint32_t>(render.object_id);
}


// PHYSICS IS NOT SERIALIZED!!!!!! it has no useful information, needs to be parsed separately
// render_id + other info can the used for that
template<>
void CustomArchive::serializeIntoBuffer<entt::registry>(const entt::registry &registry) {
	size_t sp_num_entities, sp_num_models, num_components, num_entities = 0;

	auto view = registry.view<entt::entity>();

	// number of entities
	// could not for the life of me find a function that returned it so I'll write it when I'm done
	sp_num_entities = data._sp;
	data.reserve(sizeof(size_t)); // reserve space (does not move its internal sp)
	data.copy_bytes(&num_entities, sizeof(size_t)); // to move its internal sp

	for(const auto entity : view) {
		// test speed of any_of vs all_of
		num_components = 0;

		// save the current sp
		sp_num_models = data._sp;

		// pretend to push a size_t into the data, to be overwritten later
		data.reserve(sizeof(size_t)); // reserve space (does not move its internal sp)
		data.copy_bytes(&num_components, sizeof(size_t)); // to move its internal sp

		if (registry.any_of<Position>(entity)) {
			serializeIntoBuffer<Position>(registry.get<Position>(entity));
			num_components++;
		}
		if (registry.any_of<Direction>(entity)) {
			serializeIntoBuffer<Direction>(registry.get<Direction>(entity));
			num_components++;
		}
		if (registry.any_of<Movement>(entity)) {
			serializeIntoBuffer<Movement>(registry.get<Movement>(entity));
			num_components++;
		}
		if (registry.any_of<Render>(entity)) {
			serializeIntoBuffer<Render>(registry.get<Render>(entity));
			num_components++;
		}


		data.write_bytes_at(&num_components, sizeof(size_t), sp_num_models);

		num_entities++;
	}

	data.write_bytes_at(&num_entities, sizeof(size_t), sp_num_entities);
}


template<>
void CustomArchive::serializeIntoFile<CompressionData>(FileHandler &file, CompressionData &data) {
	(void)file.write(&data.len, sizeof(data.len));
	(void)file.write(data.data, data.len);
}

template<>
void CustomArchive::deserializeFromFile<CompressionData>(FileHandler &file, CompressionData &data) {
	(void)file.read(&data.len, sizeof(data.len));
	data.data = std::malloc(data.len);
	(void)file.read(data.data, data.len);
}
