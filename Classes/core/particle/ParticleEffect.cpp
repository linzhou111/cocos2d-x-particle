#include "ParticleEffect.h"
#include "core/util/GameUtil.h"

USING_NS_CUSTOM;

void NS_CUSTOM::ParticleEffect::init(ParticleEffect* effect)
{
	emitters.clear();
	removeAllChildren();
	for (auto em : effect->emitters){
		auto emitter = ParticleEmitter::create();
		emitter->init(em);
		emitters.pushBack(emitter);
		addChild(emitter);
	}
}

Map<string, ParticleEffect*> NS_CUSTOM::ParticleEffect::particleCache;

ParticleEffect* NS_CUSTOM::ParticleEffect::createFromCache(const string name)
{
	auto p = particleCache.at(name);
	if (p == nullptr){
		auto tmp = ParticleEffect::create();
		tmp->loadEmitters(name);
		tmp->loadEmitterImages(getPathForFilename(name));
		particleCache.insert(name, tmp);
		return tmp;
	}
	else{
		auto tmp = ParticleEffect::create();
		tmp->init(p);
		return tmp;
	}
}

void NS_CUSTOM::ParticleEffect::clearCache()
{
	particleCache.clear();
}

void ParticleEffect::start()
{
	scheduleUpdate();
	for (auto emitter : emitters)
		emitter->start();
}

void ParticleEffect::reset()
{
	for (auto emitter : emitters)
		emitter->reset();
}

void NS_CUSTOM::ParticleEffect::setCompleteListener(completeListener listener)
{
	_completeListener = listener;
}

void ParticleEffect::update(float delta)
{
	if (_freeMode){
		const Vec2& pos = convertToWorldSpace(Vec2::ZERO);
		for (auto emitter : emitters){
			emitter->translate(pos.x - _lastWorldX, pos.y - _lastWorldY);
		}
		_lastWorldX = pos.x;
		_lastWorldY = pos.y;
	}
	for (auto emitter : emitters){
		emitter->update(delta);
	}
	if (isComplete()){
		if (_completeListener) _completeListener();
		removeFromParent();
	}
}

void ParticleEffect::allowCompletion()
{
	for (auto emitter : emitters)
		emitter->allowCompletion();
}

bool ParticleEffect::isComplete()
{
	for (auto emitter : emitters) {
		if (!emitter->isComplete()) return false;
	}
	return true;
}

void ParticleEffect::setDuration(int duration)
{
	for (auto emitter : emitters) {
		emitter->setContinuous(false);
		emitter->duration = duration;
		emitter->durationTimer = 0;
	}
}

void ParticleEffect::setPosition(float x, float y)
{
	Node::setPosition(x, y);
	for (auto emitter : emitters)
		emitter->setPosition(x, y);
}

void ParticleEffect::setFlip(bool flipX, bool flipY)
{
	for (auto emitter : emitters)
		emitter->setFlip(flipX, flipY);
}

void ParticleEffect::flipY()
{
	for (auto emitter : emitters)
		emitter->flipY();
}

void NS_CUSTOM::ParticleEffect::setFreeMode(bool isFree)
{
	if (_freeMode == isFree)return;
	_freeMode = isFree;
	if (_freeMode){
		const Vec2& pos = convertToWorldSpace(Vec2::ZERO);
		_lastWorldX = pos.x;
		_lastWorldY = pos.y;
	}
}

ParticleEmitter* ParticleEffect::findEmitter(string name)
{
	for (auto emitter : emitters) {
		if (emitter->getName() == name) return emitter;
	}
	return nullptr;
}

void ParticleEffect::save(ostream& output)
{
	int index = 0;
	for (auto emitter : emitters) {
		if (index++ > 0) output << "\n\n";
		emitter->save(output);
	}
}

void ParticleEffect::loadEmitters(string file)
{
	emitters.clear();
	removeAllChildren();
	string str = FileUtils::getInstance()->getStringFromFile(file);
	std::istringstream iss(str);
	string line;
	while (true) {
		auto emitter = ParticleEmitter::create();
		emitter->load(iss);
		emitters.pushBack(emitter);
		addChild(emitter);
		getline(iss, line);
		if (iss.eof()) break;
		getline(iss, line);
		if (iss.eof()) break;
	}
}

void ParticleEffect::loadEmitterImages(string path)
{
	ownsTexture = true;
	for (auto emitter : emitters){
		string imagePath = emitter->getImagePath();
		if (imagePath.empty()) continue;
		auto sprite = createSprite(path + imagePath);
		emitter->setSprite(sprite);
	}
}

BoundingBox& ParticleEffect::getBoundingBox()
{
	bounds.inf();
	for (auto emitter : emitters)
		bounds.ext(emitter->getBoundingBox());
	return bounds;
}

void ParticleEffect::scaleEffect(float scaleFactor)
{
	for (auto particleEmitter : emitters) {
		particleEmitter->getScale().setHigh(particleEmitter->getScale().getHighMin() * scaleFactor,
			particleEmitter->getScale().getHighMax() * scaleFactor);
		particleEmitter->getScale().setLow(particleEmitter->getScale().getLowMin() * scaleFactor,
			particleEmitter->getScale().getLowMax() * scaleFactor);

		particleEmitter->getVelocity().setHigh(particleEmitter->getVelocity().getHighMin() * scaleFactor,
			particleEmitter->getVelocity().getHighMax() * scaleFactor);
		particleEmitter->getVelocity().setLow(particleEmitter->getVelocity().getLowMin() * scaleFactor,
			particleEmitter->getVelocity().getLowMax() * scaleFactor);

		particleEmitter->getGravity().setHigh(particleEmitter->getGravity().getHighMin() * scaleFactor,
			particleEmitter->getGravity().getHighMax() * scaleFactor);
		particleEmitter->getGravity().setLow(particleEmitter->getGravity().getLowMin() * scaleFactor,
			particleEmitter->getGravity().getLowMax() * scaleFactor);

		particleEmitter->getWind().setHigh(particleEmitter->getWind().getHighMin() * scaleFactor,
			particleEmitter->getWind().getHighMax() * scaleFactor);
		particleEmitter->getWind().setLow(particleEmitter->getWind().getLowMin() * scaleFactor,
			particleEmitter->getWind().getLowMax() * scaleFactor);

		particleEmitter->getSpawnWidth().setHigh(particleEmitter->getSpawnWidth().getHighMin() * scaleFactor,
			particleEmitter->getSpawnWidth().getHighMax() * scaleFactor);
		particleEmitter->getSpawnWidth().setLow(particleEmitter->getSpawnWidth().getLowMin() * scaleFactor,
			particleEmitter->getSpawnWidth().getLowMax() * scaleFactor);

		particleEmitter->getSpawnHeight().setHigh(particleEmitter->getSpawnHeight().getHighMin() * scaleFactor,
			particleEmitter->getSpawnHeight().getHighMax() * scaleFactor);
		particleEmitter->getSpawnHeight().setLow(particleEmitter->getSpawnHeight().getLowMin() * scaleFactor,
			particleEmitter->getSpawnHeight().getLowMax() * scaleFactor);

		particleEmitter->getXOffsetValue().setLow(particleEmitter->getXOffsetValue().getLowMin() * scaleFactor,
			particleEmitter->getXOffsetValue().getLowMax() * scaleFactor);

		particleEmitter->getYOffsetValue().setLow(particleEmitter->getYOffsetValue().getLowMin() * scaleFactor,
			particleEmitter->getYOffsetValue().getLowMax() * scaleFactor);
	}
}

void ParticleEffect::setEmittersCleanUpBlendFunction(bool cleanUpBlendFunction)
{
	for (auto emitter : emitters) {
		emitter->setCleansUpBlendFunction(cleanUpBlendFunction);
	}
}
