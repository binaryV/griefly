#include "SpaceTurf.h"

#include "../SyncRandom.h"

#include "Item.h"
#include "Materials.h"
#include "Lattice.h"
#include "../ObjectFactory.h"
#include "../Game.h"

using namespace kv;

Space::Space()
{
    SetAtmosState(atmos::SPACE);

    SetPassable(Dir::ALL, Passable::FULL);
    transparent = true;
    
    SetFriction(0);

    SetSprite("icons/space.dmi");
    SetState("10");
    name = "Space";
}

void Space::AfterWorldCreation()
{
    quint32 value = GetRand() % 25 + 1;
    SetState(QString::number(value));
}

void Space::AttackBy(IdPtr<Item> item)
{
    if (IdPtr<Rod> rod = item)
    {
        rod->Delete();
        Create<IOnMapObject>(Lattice::GetTypeStatic(), GetOwner());
    }
}
