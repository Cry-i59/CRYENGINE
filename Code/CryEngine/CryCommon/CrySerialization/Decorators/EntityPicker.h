// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#pragma once

#include <CrySerialization/IArchive.h>
#include <CrySerialization/Decorators/ActionButton.h>
#include <CryEntitySystem/IEntitySystem.h>

namespace Serialization
{

struct IEntityPicker
{
	virtual void OnEntityPicked(IEntity& entity) = 0;
	virtual bool CanPickEntity(IEntity& entity) = 0;
};

struct EntityPickerButton final : public IEntityPicker
{
	struct PickedEntity
	{
		CryGUID guid;

		bool Serialize(Serialization::IArchive& ar)
		{
			if (ar.isEdit())
			{
				if (EntityId id = gEnv->pEntitySystem->FindEntityByGuid(guid))
				{
					if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id))
					{
						string name = pEntity->GetName();
						ar(name, "targetName", "!Target");
					}
				}
				return true;
			}
			else
			{
				return ar(guid, "targetName", "!Target");
			}
		}
		
		bool operator==(const PickedEntity& rhs) const
		{
			return guid == rhs.guid;
		}
	};

	// IEntityPicker
	virtual void OnEntityPicked(IEntity& entity) override
	{
		pickedEntities.emplace_back(PickedEntity{ entity.GetGuid() });
	}

	virtual bool CanPickEntity(IEntity& entity) override
	{
		return allowPickCallback == nullptr || allowPickCallback(entity);
	}
	// ~IEntityPicker

	inline bool operator==(const EntityPickerButton &rhs) const
	{
		if (pickedEntities.size() != rhs.pickedEntities.size())
		{
			return false;
		}

		return std::equal(pickedEntities.begin(), pickedEntities.end(), rhs.pickedEntities.begin());
	}

	void IterateEntities(const std::function<void(IEntity& entity)> callback) const
	{
		for (const PickedEntity& pickedEntity : pickedEntities)
		{
			const EntityId id = gEnv->pEntitySystem->FindEntityByGuid(pickedEntity.guid);
			if (id == INVALID_ENTITYID)
				continue;

			IEntity* pEntity = gEnv->pEntitySystem->GetEntity(id);
			if (pEntity == nullptr)
				continue;

			callback(*pEntity);
		}
	}

	std::vector<PickedEntity> pickedEntities;
	std::function<bool(IEntity&)> allowPickCallback;
};

inline bool Serialize(Serialization::IArchive& ar, EntityPickerButton& button, const char* name, const char* label)
{
	if (ar.isEdit())
	{
		if (ar.openBlock(name, label))
		{
			ar(Serialization::SStruct::forEdit(static_cast<Serialization::IEntityPicker&>(button)), "picker", "^Pick");
			ar(Serialization::ActionButton([&button]
			{
				button.pickedEntities.clear();
			}), "picker", "^Clear");

			ar(button.pickedEntities, "pickedEntities", "Entities");

			ar.closeBlock();
			return true;
		}
	}

	return ar(button.pickedEntities, "pickedEntities");
}

}