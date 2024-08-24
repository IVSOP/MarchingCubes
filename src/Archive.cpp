#include "Archive.hpp"
#include "Phys.hpp" // JPH::Vec3 and JPH::Quat

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

#include "Logs.hpp"

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
void CustomArchive::serializeIntoBuffer<JPH::Vec3>(const JPH::Vec3 &val) {
	data.reserve(3 * sizeof(float));
	float x = val.GetX(), y = val.GetY(), z = val.GetZ();
	data.copy_bytes(&x, sizeof(float));
	data.copy_bytes(&y, sizeof(float));
	data.copy_bytes(&z, sizeof(float));
}

template<>
void CustomArchive::serializeIntoBuffer<JPH::Quat>(const JPH::Quat &val) {
	data.reserve(4 * sizeof(float));
	float x = val.GetX(), y = val.GetY(), z = val.GetZ(), w = val.GetW();
	data.copy_bytes(&x, sizeof(float));
	data.copy_bytes(&y, sizeof(float));
	data.copy_bytes(&z, sizeof(float));
	data.copy_bytes(&w, sizeof(float));
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
void CustomArchive::serializeIntoBuffer<Physics>(const Physics &phys) {
	Component component_id = Component::Physics;
	serializeIntoBuffer<Component>(component_id);
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
		if (registry.any_of<Physics>(entity)) {
			serializeIntoBuffer<Physics>(registry.get<Physics>(entity));
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

template<>
void CustomArchive::deserializeFromBuffer<size_t>(size_t &val) {
	data.extract_bytes(&val, sizeof(size_t), read_sp);
	read_sp += sizeof(size_t);
}

template<>
void CustomArchive::deserializeFromBuffer<Component>(Component &val) {
	data.extract_bytes(&val, sizeof(Component), read_sp);
	read_sp += sizeof(Component);
}

template<>
void CustomArchive::deserializeFromBuffer<uint32_t>(uint32_t &val) {
	data.extract_bytes(&val, sizeof(uint32_t), read_sp);
	read_sp += sizeof(uint32_t);
}

// template<>
// void CustomArchive::deserializeFromBuffer<float>(float &val) {
// 	data.extract_bytes(&val, sizeof(float), read_sp);
// 	read_sp += sizeof(float);
// }

template<>
void CustomArchive::deserializeFromBuffer<GLfloat>(GLfloat &val) {
	data.extract_bytes(&val, sizeof(GLfloat), read_sp);
	read_sp += sizeof(GLfloat);
}

template<>
void CustomArchive::deserializeFromBuffer<bool>(bool &val) {
	data.extract_bytes(&val, sizeof(bool), read_sp);
	read_sp += sizeof(bool);
}

template<>
void CustomArchive::deserializeFromBuffer<glm::vec3>(glm::vec3 &val) {
	deserializeFromBuffer<GLfloat>(val.x);
	deserializeFromBuffer<GLfloat>(val.y);
	deserializeFromBuffer<GLfloat>(val.z);
}

template<>
void CustomArchive::deserializeFromBuffer<Position>(Position &pos) {
	deserializeFromBuffer<glm::vec3>(pos.pos);
}

template<>
void CustomArchive::deserializeFromBuffer<Direction>(Direction &dir) {
	deserializeFromBuffer<GLfloat>(dir.pitch);
	deserializeFromBuffer<GLfloat>(dir.yaw);
	deserializeFromBuffer<glm::vec3>(dir.front);
	deserializeFromBuffer<glm::vec3>(dir.worldup);
	deserializeFromBuffer<glm::vec3>(dir.up);
	deserializeFromBuffer<glm::vec3>(dir.right);
}

template<>
void CustomArchive::deserializeFromBuffer<Movement>(Movement &mov) {
	deserializeFromBuffer<GLfloat>(mov.speed);
	deserializeFromBuffer<bool>(mov.speedup);
}

template<>
void CustomArchive::deserializeFromBuffer<Render>(Render &render) {
	deserializeFromBuffer<uint32_t>(render.object_id);
}

template<>
void CustomArchive::deserializeFromBuffer<JPH::Vec3>(JPH::Vec3 &vec) {
	float x, y, z;

	deserializeFromBuffer<float>(x);
	deserializeFromBuffer<float>(y);
	deserializeFromBuffer<float>(z);

	vec.SetX(x);
	vec.SetY(y);
	vec.SetZ(z);
}

template<>
void CustomArchive::deserializeFromBuffer<JPH::Quat>(JPH::Quat &vec) {
	float x, y, z, w;

	deserializeFromBuffer<float>(x);
	deserializeFromBuffer<float>(y);
	deserializeFromBuffer<float>(z);
	deserializeFromBuffer<float>(w);

	vec.SetX(x);
	vec.SetY(y);
	vec.SetZ(z);
	vec.SetW(w);
}

template<>
void CustomArchive::deserializeFromBuffer<entt::registry>(entt::registry &registry) {
	size_t num_entities;
	deserializeFromBuffer<size_t>(num_entities);

	size_t num_components;
	Component component_id;
	for (size_t i = 0; i < num_entities; i++) {
		entt::entity entity = registry.create();
		deserializeFromBuffer<size_t>(num_components);
		for (size_t c = 0; c < num_components; c++) {
			deserializeFromBuffer<Component>(component_id);
			// cursed but doing things like Position &position = registry.emplace<Position>(entity); doesnt work for some reason
			switch (component_id) {
				case Component::Position:
					{

						Position position;
						deserializeFromBuffer<Position>(position);
						registry.emplace<Position>(entity, position.pos);
					}
					break;
				case Component::Direction:
					{
						Direction direction;
						deserializeFromBuffer<Direction>(direction);
						registry.emplace<Direction>(entity, direction.pitch, direction.yaw, direction.front, direction.worldup, direction.up, direction.right);
					}
					break;
				case Component::Movement:
					{
						Movement movement;
						deserializeFromBuffer<Movement>(movement);
						registry.emplace<Movement>(entity, movement.speed, movement.speedup);
					}
					break;
				case Component::Physics:
					{
						// Physics phys;
						// deserializeFromBuffer<Physics>(phys);
						registry.emplace<Physics>(entity);
					}
					break;
				case Component::Render:
					{
						Render render;
						deserializeFromBuffer<Render>(render);
						registry.emplace<Render>(entity, render.object_id);
					}
					break;
				default:
					print_error("Error loading component: ID " + std::to_string(static_cast<uint32_t>(component_id)) + " not recognized");
					exit(1);
			}
		}
	}
}
