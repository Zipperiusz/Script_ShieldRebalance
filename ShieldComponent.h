#pragma once
class ShieldComponent : public eCEngineComponentBase {
public: virtual void	Process(void);
public: virtual			~ShieldComponent(void);
private:
	static bTPropertyObject<ShieldComponent, eCEngineComponentBase> ms_PropertyObjectInstance_ShieldComponent;

public:
	ShieldComponent();

private:
	ShieldComponent(ShieldComponent const&);
	ShieldComponent const& operator = (ShieldComponent const&);
};

ShieldComponent::~ShieldComponent(void) {}
bTPropertyObject<ShieldComponent, eCEngineComponentBase> ShieldComponent::ms_PropertyObjectInstance_ShieldComponent(GETrue);
static bCAccessorCreator shieldComponent(bTClassName<ShieldComponent>::GetUnmangled());
