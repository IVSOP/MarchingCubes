#include "Archive.hpp"

// reads size_t from file with the total size,
// reads N bytes from file using that values as the number of bytes
// places everything in this->data
CustomArchive::CustomArchive(FileHandler &file)
: data(1)
{
	size_t len;
	file.read(&len, sizeof(size_t));
	data.reserve(len);
	void *buf = std::malloc(len);
	// TODO
	(void)file.read(buf, len);
	data.copy_bytes(buf, len);
	free(buf);
}

template<>
void CustomArchive::serializeIntoBuffer<uint32_t>(const uint32_t &val) { // reference is cursed here but whatever
	data.reserve(sizeof(uint32_t));
	data.copy_bytes(&val, sizeof(uint32_t));
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
// render_id + other info can the uesd for that
template<>
void CustomArchive::serializeIntoBuffer<entt::registry>(const entt::registry &registry) {
	size_t sp, sp_total;
	auto view = registry.view<entt::entity>();
	for(const auto entity : view) {
		// test speed of any_of vs all_of

		// save the current sp
		sp = data._sp;

		// pretend to push a size_t into the data, to be overwritten later
		data.reserve(sizeof(size_t));
		data.copy_bytes(&sp, sizeof(size_t));

		if (registry.any_of<Position>(entity)) {
			serializeIntoBuffer<Position>(registry.get<Position>(entity));
		}
		if (registry.any_of<Direction>(entity)) {
			serializeIntoBuffer<Direction>(registry.get<Direction>(entity));
		}
		if (registry.any_of<Movement>(entity)) {
			serializeIntoBuffer<Movement>(registry.get<Movement>(entity));
		}
		if (registry.any_of<Render>(entity)) {
			serializeIntoBuffer<Render>(registry.get<Render>(entity));
		}

		// now check sp again
		sp_total = data._sp - sp;

		// now we know how many bytes this entity took
		data.write_bytes_at(&sp_total, sizeof(size_t), sp);
	}
}

// void CustomArchive::deSerializeIntoRegistry(entt::registry registry) const {

// }
