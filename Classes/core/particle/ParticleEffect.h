#ifndef __PARTICLE_EFFECT_H__
#define __PARTICLE_EFFECT_H__

#include <string>
#include <iostream>
#include <sstream>
#include "cocos2d.h"
#include "ParticleEmitter.h"
#include "core/util/GameDefine.h"

USING_NS_CC;
using std::string;

NS_CUSTOM_BEGIN


class ParticleEffect :public Node{
private:
	//»º´æ
	static Map<string, ParticleEffect*>particleCache;
	cocos2d::Vector<ParticleEmitter*> emitters;
	BoundingBox bounds;
	bool ownsTexture;
	virtual bool init(){ return true; }
	typedef std::function<void()> completeListener;
	completeListener _completeListener;
	bool _freeMode;
	float _lastWorldX, _lastWorldY;
public:
	//»º´æ´´½¨
	static ParticleEffect* createFromCache(const string name);
	static void clearCache();

	ParticleEffect() :_completeListener(nullptr) {}

	virtual void init(ParticleEffect* effect);

	virtual void start();

	virtual void reset();

	virtual void setCompleteListener(completeListener listener);

	virtual void update(float delta) override;

	virtual void allowCompletion();

	virtual bool isComplete();

	virtual void setDuration(int duration);

	virtual void setPosition(float x, float y);

	virtual void setFlip(bool flipX, bool flipY);

	virtual void flipY();

	virtual void setFreeMode(bool isFree);

	virtual cocos2d::Vector<ParticleEmitter*>& getEmitters() {
		return emitters;
	}

	/** Returns the emitter with the specified name, or null. */
	virtual ParticleEmitter* findEmitter(string name);

	virtual void save(ostream& output);

	virtual void loadEmitters(string file);

	virtual void loadEmitterImages(string path = PARTICLE_IMAGE_PATH);

	/** Returns the bounding box for all active particles. z axis will always be zero. */
	virtual BoundingBox& getBoundingBox();

	virtual void scaleEffect(float scaleFactor);

	/** Sets the {@link com.badlogic.gdx.graphics.g2d.ParticleEmitter#setCleansUpBlendFunction(boolean) cleansUpBlendFunction}
	* parameter on all {@link com.badlogic.gdx.graphics.g2d.ParticleEmitter ParticleEmitters} currently in this ParticleEffect.
	* <p>
	* IMPORTANT: If set to false and if the next object to use this Batch expects alpha blending, you are responsible for setting
	* the Batch's blend function to (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) before that next object is drawn.
	* @param cleanUpBlendFunction */
	virtual void setEmittersCleanUpBlendFunction(bool cleanUpBlendFunction);

	CREATE_FUNC(ParticleEffect);

};

NS_CUSTOM_END

#endif
