#include "Core/Headers/Entity.h"

 size_t EntityMemoryPool::m_maxEntities{};


int Old_Entity::getID() const
{
	return m_id;
}


const std::string& Old_Entity::getTag() const
{
	return EntityMemoryPool::Instance().m_entityTags[m_id];
}

void Entity::setRotation(Vec3f axis, float angle)
{
	m_currentRotationAxis = axis;
	m_currentRotationAngle = angle;
}

void Entity::imguiDraw()
{
	float imgui_Pos[3]{ m_position.x, m_position.y, m_position.z };
	float imgui_RotAxis[3]{ m_currentRotationAxis.x, m_currentRotationAxis.y, m_currentRotationAxis.z };
	float imgui_RotAngle = m_currentRotationAngle;


	ImGui::DragFloat3("Position", imgui_Pos);
	ImGui::DragFloat3("Rotation Axis", imgui_RotAxis);
	ImGui::DragFloat("Angle", &imgui_RotAngle);

}

MeshEntity::MeshEntity(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> material)
{
	m_meshes.push_back(mesh);
	m_materials.push_back(material);
}

void MeshEntity::imguiDraw()
{
	Entity::imguiDraw();
	float imgui_Scale[3]{ m_scale.x, m_scale.y, m_scale.z };

	ImGui::DragFloat3("Scale", imgui_Scale);
}

void MeshEntity::render()
{
	for (auto& mat : m_materials)
	{
	}

	for (auto& mesh : m_meshes)
	{

	}
}
