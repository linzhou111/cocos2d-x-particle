#ifndef __PARTICLE_EMITTER_H__
#define __PARTICLE_EMITTER_H__

#include <string>
#include <iostream>
#include "cocos2d.h"
#include "core/util/GameDefine.h"

USING_NS_CC;
using std::string;
using std::istream;
using std::ostream;

NS_CUSTOM_BEGIN

class Particle;

class ParticleValue {
public:
	friend class ParticleEmitter;

	ParticleValue() : active(false), alwaysActive(false){}

	virtual void setAlwaysActive(bool alwaysActive) {
		this->alwaysActive = alwaysActive;
	}

	virtual bool isAlwaysActive() {
		return alwaysActive;
	}

	virtual bool isActive() {
		return alwaysActive || active;
	}

	virtual void setActive(bool active) {
		this->active = active;
	}

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(ParticleValue& value);
protected:
	bool active;
	bool alwaysActive;
};

class NumericValue :public ParticleValue {
public:
	friend class ParticleEmitter;

	NumericValue() :value(0){}

	virtual float getValue() {
		return value;
	}

	virtual void setValue(float value) {
		this->value = value;
	}

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(NumericValue& value);
private:
	float value;
};

class RangedNumericValue :public ParticleValue {
public:
	friend class ParticleEmitter;

	RangedNumericValue() :lowMin(0), lowMax(0){}

	virtual float newLowValue() {
		return lowMin <= lowMax ? random(lowMin, lowMax) : random(lowMax, lowMin);
	}

	virtual void setLow(float value) {
		lowMin = value;
		lowMax = value;
	}

	virtual void setLow(float min, float max) {
		lowMin = min;
		lowMax = max;
	}

	virtual float getLowMin() {
		return lowMin;
	}

	virtual void setLowMin(float lowMin) {
		this->lowMin = lowMin;
	}

	virtual float getLowMax() {
		return lowMax;
	}

	virtual void setLowMax(float lowMax) {
		this->lowMax = lowMax;
	}

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(RangedNumericValue& value);
private:
	float lowMin, lowMax;
};

class ScaledNumericValue :public RangedNumericValue {
public:
	friend class ParticleEmitter;

	ScaledNumericValue() :highMin(0), highMax(0), relative(false){}

	virtual float newHighValue() {
		return highMin <= highMax ? random(highMin, highMax) : random(highMax, highMin);
	}

	virtual void setHigh(float value) {
		highMin = value;
		highMax = value;
	}

	virtual void setHigh(float min, float max) {
		highMin = min;
		highMax = max;
	}

	virtual float getHighMin() {
		return highMin;
	}

	virtual void setHighMin(float highMin) {
		this->highMin = highMin;
	}

	virtual float getHighMax() {
		return highMax;
	}

	virtual void setHighMax(float highMax) {
		this->highMax = highMax;
	}

	virtual float_array getScaling() {
		return scaling;
	}

	virtual void setScaling(float_array values) {
		this->scaling = values;
	}

	virtual float_array getTimeline() {
		return timeline;
	}

	virtual void setTimeline(float_array timeline) {
		this->timeline = timeline;
	}

	virtual bool isRelative() {
		return relative;
	}

	virtual void setRelative(bool relative) {
		this->relative = relative;
	}

	virtual float getScale(float percent);

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(ScaledNumericValue& value);
private:
	float_array scaling = float_array{ 1.0f };
	float_array timeline = float_array{ 0.0f };
	float highMin, highMax;
	bool relative;
};

class GradientColorValue :public ParticleValue {
public:
	friend class ParticleEmitter;
	GradientColorValue(){
		alwaysActive = true;
	}

	float_array getTimeline() {
		return timeline;
	}

	virtual void setTimeline(float_array timeline) {
		this->timeline = timeline;
	}

	/** @return the r, g and b values for every timeline position */
	virtual float_array& getColors() {
		return colors;
	}

	/** @param colors the r, g and b values for every timeline position */
	virtual void setColors(float_array colors) {
		this->colors = colors;
	}

	virtual float_array& getColor(float percent);

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(GradientColorValue& value);
private:
	static float_array temp;

	float_array colors = float_array{ 1.0f, 1.0f, 1.0f };
	float_array timeline = float_array{ 0.0f };
};

enum SpawnShape {
	point, line, square, ellipse
};

static std::map<string, SpawnShape> SpawnShapeMap{
	{ "point", point }, { "line", line }, { "square", square }, { "ellipse", ellipse }
};

enum SpawnEllipseSide {
	both, top, bottom
};

static std::map<string, SpawnEllipseSide> SpawnEllipseSideMap{
	{ "both", both }, { "top", top }, { "bottom", bottom }
};

class SpawnShapeValue : public ParticleValue {
public:
	friend class ParticleEmitter;

	SpawnShapeValue() :edges(false){}

	SpawnShape getShape() {
		return shape;
	}

	void setShape(SpawnShape shape) {
		this->shape = shape;
	}

	bool isEdges() {
		return edges;
	}

	void setEdges(bool edges) {
		this->edges = edges;
	}


	virtual SpawnEllipseSide getSide() {
		return side;
	}

	virtual void setSide(SpawnEllipseSide side) {
		this->side = side;
	}

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

	virtual void load(SpawnShapeValue& value);


protected:
	SpawnShape shape = SpawnShape::point;
	bool edges;
	SpawnEllipseSide side = SpawnEllipseSide::both;
};

class BoundingBox{
public:
	Vec3 min;
	Vec3 max;

	BoundingBox() {
		clr();
	}

	BoundingBox(const BoundingBox& bounds) {
		set(bounds);
	}

	/** Sets the minimum and maximum vector to zeros.
	* @return This bounding box for chaining. */
	virtual BoundingBox clr();

	/** Sets the minimum and maximum vector to positive and negative infinity.
	*
	* @return This bounding box for chaining. */
	virtual BoundingBox& inf();

	/** Sets the given bounding box.
	*
	* @param bounds The bounds.
	* @return This bounding box for chaining. */
	virtual BoundingBox& set(const BoundingBox& bounds) {
		return set(bounds.min, bounds.max);
	}

	/** Sets the given minimum and maximum vector.
	*
	* @param minimum The minimum vector
	* @param maximum The maximum vector
	* @return This bounding box for chaining. */
	virtual BoundingBox& set(const Vec3& minimum, const Vec3& maximum);

	/** Extends the bounding box by the given vector.
	*
	* @param x The x-coordinate
	* @param y The y-coordinate
	* @param z The z-coordinate
	* @return This bounding box for chaining. */
	virtual BoundingBox& ext(float x, float y, float z);

	/** Extends this bounding box by the given bounding box.
	*
	* @param a_bounds The bounding box
	* @return This bounding box for chaining. */
	virtual BoundingBox& ext(const BoundingBox& a_bounds);

private:
	static const Vec3 tmpVector;
	Vec3 cnt;
	Vec3 dim;

};

class Particle {
public:
	friend class ParticleEmitter;
	Particle() :
		//_flipX(false), _flipY(false),
		_scale(0),
		_rotation(0),
		_opacity(255),
		life(0), currentLife(0),
		scale(0), scaleDiff(0),
		rotation(0), rotationDiff(0),
		velocity(0), velocityDiff(0),
		angle(0), angleDiff(0),
		angleCos(0), angleSin(0),
		transparency(0), transparencyDiff(0),
		wind(0), windDiff(0),
		gravity(0), gravityDiff(0)
	{}

	virtual void setPosition(float x, float y);
	virtual void translate(float dx, float dy);
protected:
	Vec2 _position;
	float _scale;
	float _rotation;
	Color3B _color;
	GLubyte _opacity;

	int life, currentLife;
	float scale, scaleDiff;
	float rotation, rotationDiff;
	float velocity, velocityDiff;
	float angle, angleDiff;
	float angleCos, angleSin;
	float transparency, transparencyDiff;
	float wind, windDiff;
	float gravity, gravityDiff;
	float_array tint = float_array{ 0, 0, 0 };
};

class ParticlePool{
public:
	ParticlePool() : _pool(nullptr), _size(0), _capacity(0){}
	~ParticlePool();
	static ParticlePool* getInstance();
	void init(int capacity);
	Particle* getFromPool();
	void freeToPool(Particle* particle);
	void clearPool();
private:
	static ParticlePool* _instance;
	Particle** _pool;
	int _size;
	int _capacity;
};

class ParticleEmitter : public Node{
public:
	static const int UPDATE_SCALE = 1 << 0;
	static const int UPDATE_ANGLE = 1 << 1;
	static const int UPDATE_ROTATION = 1 << 2;
	static const int UPDATE_VELOCITY = 1 << 3;
	static const int UPDATE_WIND = 1 << 4;
	static const int UPDATE_GRAVITY = 1 << 5;
	static const int UPDATE_TINT = 1 << 6;

	static Color3B tempColor;

	float duration = 1, durationTimer = 0;

	ParticleEmitter() :
		accumulator(0),
		sprite(nullptr),
		_spriteWidth(0), _spriteHeight(0),
		particles(nullptr),
		minParticleCount(0), maxParticleCount(0),
		dx(0), dy(0),
		activeCount(0),
		active(nullptr),
		firstUpdate(false),
		_flipX(false), _flipY(false),
		updateFlags(0),
		_allowCompletion(false),
		emission(0), emissionDiff(0), emissionDelta(0),
		lifeOffset(0), lifeOffsetDiff(0),
		life(0), lifeDiff(0),
		spawnWidth(0), spawnWidthDiff(0),
		spawnHeight(0), spawnHeightDiff(0),
		delay(0), delayTimer(0),
		attached(false),
		continuous(false),
		aligned(false),
		behind(false),
		_allocatedParticles(0),
		_blendFunc(BlendFunc::ALPHA_NON_PREMULTIPLIED),
		_quads(nullptr),
		_indices(nullptr),
		_VAOname(0)
	{
		memset(_buffersVBO, 0, sizeof(_buffersVBO));
		initialize();
	}

	virtual ~ParticleEmitter(){
		CC_SAFE_RELEASE_NULL(sprite);
		CC_SAFE_FREE(_quads);
		CC_SAFE_FREE(_indices);
		glDeleteBuffers(2, &_buffersVBO[0]);
		if (Configuration::getInstance()->supportsShareableVAO())
		{
			glDeleteVertexArrays(1, &_VAOname);
			GL::bindVAO(0);
		}
	}

	ParticleEmitter(ParticleEmitter* emitter);

	void init(ParticleEmitter* emitter);

	void draw(Renderer *renderer, const Mat4 &transform, uint32_t flags);

	void setMaxParticleCount(int maxParticleCount);

	void addParticle();

	void addParticles(int count);

	void update(float delta);

	void start();

	void reset();

	void restart();

	void setPosition(float x, float y);

	void translate(float x, float y);

	void setFlip(bool flipX, bool flipY);

	void setSprite(Sprite* sprite);

	/** Ignores the {@link #setContinuous(boolean) continuous} setting until the emitter is started again. This allows the emitter
	* to stop smoothly. */
	void allowCompletion() {
		this->_allowCompletion = true;
		durationTimer = duration;
	}

	CREATE_FUNC(ParticleEmitter);

	virtual bool init(){ return true; }

	Particle* newParticle(Sprite* sprite);

	void activateParticle(int index);

	bool updateParticle(Particle* particle, float delta, int deltaMillis);

	Sprite* getSprite() {
		return sprite;
	}

	string getName() {
		return name;
	}

	void setName(string name) {
		this->name = name;
	}

	ScaledNumericValue& getLife() {
		return lifeValue;
	}

	ScaledNumericValue& getScale() {
		return scaleValue;
	}

	ScaledNumericValue& getRotation() {
		return rotationValue;
	}

	GradientColorValue& getTint() {
		return tintValue;
	}

	ScaledNumericValue& getVelocity() {
		return velocityValue;
	}

	ScaledNumericValue& getWind() {
		return windValue;
	}

	ScaledNumericValue& getGravity() {
		return gravityValue;
	}

	ScaledNumericValue& getAngle() {
		return angleValue;
	}

	ScaledNumericValue& getEmission() {
		return emissionValue;
	}

	ScaledNumericValue& getTransparency() {
		return transparencyValue;
	}

	RangedNumericValue& getDuration() {
		return durationValue;
	}

	RangedNumericValue& getDelay() {
		return delayValue;
	}

	ScaledNumericValue& getLifeOffset() {
		return lifeOffsetValue;
	}

	RangedNumericValue& getXOffsetValue() {
		return xOffsetValue;
	}

	RangedNumericValue& getYOffsetValue() {
		return yOffsetValue;
	}

	ScaledNumericValue& getSpawnWidth() {
		return spawnWidthValue;
	}

	ScaledNumericValue& getSpawnHeight() {
		return spawnHeightValue;
	}

	SpawnShapeValue& getSpawnShape() {
		return spawnShapeValue;
	}

	bool isAttached() {
		return attached;
	}

	void setAttached(bool attached) {
		this->attached = attached;
	}

	bool isContinuous() {
		return continuous;
	}

	void setContinuous(bool continuous) {
		this->continuous = continuous;
	}

	bool isAligned() {
		return aligned;
	}

	void setAligned(bool aligned) {
		this->aligned = aligned;
	}

	bool isAdditive() {
		return additive;
	}

	void setAdditive(bool additive) {
		this->additive = additive;
	}

	/** @return Whether this ParticleEmitter automatically returns the {@link com.badlogic.gdx.graphics.g2d.Batch Batch}'s blend
	*         function to the alpha-blending default (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) when done drawing. */
	bool cleansUpBlendFunction() {
		return _cleansUpBlendFunction;
	}

	/** Set whether to automatically return the {@link com.badlogic.gdx.graphics.g2d.Batch Batch}'s blend function to the
	* alpha-blending default (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) when done drawing. Is true by default. If set to false, the
	* Batch's blend function is left as it was for drawing this ParticleEmitter, which prevents the Batch from being flushed
	* repeatedly if consecutive ParticleEmitters with the same additive or pre-multiplied alpha state are drawn in a row.
	* <p>
	* IMPORTANT: If set to false and if the next object to use this Batch expects alpha blending, you are responsible for setting
	* the Batch's blend function to (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) before that next object is drawn.
	* @param cleansUpBlendFunction */
	void setCleansUpBlendFunction(bool cleansUpBlendFunction) {
		this->_cleansUpBlendFunction = cleansUpBlendFunction;
	}

	bool isBehind() {
		return behind;
	}

	void setBehind(bool behind) {
		this->behind = behind;
	}

	bool isPremultipliedAlpha() {
		return premultipliedAlpha;
	}

	void setPremultipliedAlpha(bool premultipliedAlpha) {
		this->premultipliedAlpha = premultipliedAlpha;
	}

	int getMinParticleCount() {
		return minParticleCount;
	}

	void setMinParticleCount(int minParticleCount) {
		this->minParticleCount = minParticleCount;
	}

	int getMaxParticleCount() {
		return maxParticleCount;
	}

	bool isComplete();

	float getPercentComplete();

	int getActiveCount() {
		return activeCount;
	}

	string getImagePath() {
		return imagePath;
	}

	void setImagePath(string imagePath) {
		this->imagePath = imagePath;
	}

	void flipY();

	/** Returns the bounding box for all active particles. z axis will always be zero. */
	BoundingBox& getBoundingBox() {
		bounds.inf();
		for (int i = 0; i < maxParticleCount; i++)
			if (active[i]) {
				// 				const auto& r = particles.at(i)->getBoundingBox();
				// 				bounds.ext(r.getMinX(), r.getMinY(), 0);
				// 				bounds.ext(r.getMaxX(), r.getMaxY(), 0);
			}

		return bounds;
	}

	virtual ostream& save(ostream& output);

	virtual void load(istream& reader);

private:
	RangedNumericValue delayValue;
	ScaledNumericValue lifeOffsetValue;
	RangedNumericValue durationValue;
	ScaledNumericValue lifeValue;
	ScaledNumericValue emissionValue;
	ScaledNumericValue scaleValue;
	ScaledNumericValue rotationValue;
	ScaledNumericValue velocityValue;
	ScaledNumericValue angleValue;
	ScaledNumericValue windValue;
	ScaledNumericValue gravityValue;
	ScaledNumericValue transparencyValue;
	GradientColorValue tintValue;
	ScaledNumericValue xOffsetValue;
	ScaledNumericValue yOffsetValue;
	ScaledNumericValue spawnWidthValue;
	ScaledNumericValue spawnHeightValue;
	SpawnShapeValue spawnShapeValue;

	float accumulator;
	Sprite* sprite;
	float _spriteWidth, _spriteHeight;
	Particle** particles;
	int minParticleCount, maxParticleCount;
	float dx, dy;
	string name;
	string imagePath;
	int activeCount;
	bool* active;
	bool firstUpdate;
	bool _flipX, _flipY;
	int updateFlags;
	bool _allowCompletion;
	BoundingBox bounds;

	int emission, emissionDiff, emissionDelta;
	int lifeOffset, lifeOffsetDiff;
	int life, lifeDiff;
	float spawnWidth, spawnWidthDiff;
	float spawnHeight, spawnHeightDiff;

	float delay, delayTimer;

	bool attached;
	bool continuous;
	bool aligned;
	bool behind;
	bool additive = true;
	bool premultipliedAlpha = false;
	bool _cleansUpBlendFunction = true;

	int _allocatedParticles;
	BlendFunc _blendFunc;

	V3F_C4B_T2F_Quad    *_quads;        // quads to be rendered
	GLushort            *_indices;      // indices
	GLuint              _VAOname;
	GLuint              _buffersVBO[2]; //0: vertex  1: indices

	QuadCommand _quadCommand;           // quad command

	/** initializes the indices for the vertices*/
	void initIndices();

	/** initializes the texture with a rectangle measured Points */
	void initTexCoordsWithRect(const Rect& rect);

	/** Updates texture coords */
	void updateTexCoords();

	void updateParticleQuads();

	void setupVBOandVAO();

	void setupVBO();

	void postStep();

	void updateBlendFunc();

	void initGLProgramState();

	void initialize();

	inline void updatePosWithParticle(V3F_C4B_T2F_Quad *quad, Particle* particle, float spriteW, float spriteH);

};



static string readString(string line);

static string readString(istream& reader, string name);

static bool readBoolean(string line);

static bool readBoolean(istream& reader, string name);

static int readInt(istream& reader, string name);

static float readFloat(istream& reader, string name);

static float randomf(float a, float b);

NS_CUSTOM_END
#endif