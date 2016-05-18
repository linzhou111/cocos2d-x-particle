#include "ParticleEmitter.h"
#include "core/util/GameUtil.h"

USING_NS_CUSTOM;

float_array GradientColorValue::temp = { 0, 0, 0, 0 };

float_array& GradientColorValue::getColor(float percent)
{
	int startIndex = 0, endIndex = -1;
	int n = timeline.size();
	for (int i = 1; i < n; i++) {
		float t = timeline[i];
		if (t > percent) {
			endIndex = i;
			break;
		}
		startIndex = i;
	}
	float startTime = timeline[startIndex];
	startIndex *= 3;
	float r1 = colors[startIndex];
	float g1 = colors[startIndex + 1];
	float b1 = colors[startIndex + 2];
	if (endIndex == -1) {
		temp[0] = r1;
		temp[1] = g1;
		temp[2] = b1;
		return temp;
	}
	float factor = (percent - startTime) / (timeline[endIndex] - startTime);
	endIndex *= 3;
	temp[0] = r1 + (colors[endIndex] - r1) * factor;
	temp[1] = g1 + (colors[endIndex + 1] - g1) * factor;
	temp[2] = b1 + (colors[endIndex + 2] - b1) * factor;
	return temp;
}

ostream& GradientColorValue::save(ostream& output)
{
	ParticleValue::save(output);
	if (!active) return output;
	output << "colorsCount: " << colors.size() << "\n";
	for (int i = 0; i != colors.size(); i++)
		output << "colors" << i << ": " << colors[i] << "\n";
	output << "timelineCount: " << timeline.size() << "\n";
	for (int i = 0; i != timeline.size(); i++)
		output << "timeline" << i << ": " << timeline[i] << "\n";
	return output;
}

void GradientColorValue::load(GradientColorValue& value)
{
	ParticleValue::load(value);
	colors.clear();
	for (float v : value.colors){
		colors.push_back(v);
	}
	timeline.clear();
	for (float v : value.timeline){
		timeline.push_back(v);
	}
}

void GradientColorValue::load(istream& reader)
{
	ParticleValue::load(reader);
	if (!active) return;
	colors.clear();
	int colorsCount = readInt(reader, "colorsCount");
	for (int i = 0; i < colorsCount; i++){
		colors.push_back(readFloat(reader, "colors" + i));
	}
	timeline.clear();
	int timelineCount = readInt(reader, "timelineCount");
	for (int i = 0; i < timelineCount; i++){
		timeline.push_back(readFloat(reader, "timeline" + i));
	}
}

void Particle::setPosition(float x, float y)
{
	_position.x = x;
	_position.y = y;
}

void Particle::translate(float dx, float dy)
{
	_position.x += dx;
	_position.y += dy;
}

BoundingBox BoundingBox::clr()
{
	min.set(0, 0, 0);
	max.set(0, 0, 0);
	return set(min, max);
}

BoundingBox& BoundingBox::inf()
{
	min.set(0x7fff, 0x7fff, 0x7fff);
	max.set(-0x7fff, -0x7fff, -0x7fff);
	cnt.set(0, 0, 0);
	dim.set(0, 0, 0);
	return *this;
}

BoundingBox& BoundingBox::set(const Vec3& minimum, const Vec3& maximum)
{
	min.set(minimum.x < maximum.x ? minimum.x : maximum.x, minimum.y < maximum.y ? minimum.y : maximum.y,
		minimum.z < maximum.z ? minimum.z : maximum.z);
	max.set(minimum.x > maximum.x ? minimum.x : maximum.x, minimum.y > maximum.y ? minimum.y : maximum.y,
		minimum.z > maximum.z ? minimum.z : maximum.z);
	cnt.set(min);
	cnt.add(max);
	cnt.scale(0.5f);
	dim.set(max);
	dim.subtract(min);
	return *this;
}

BoundingBox& BoundingBox::ext(float x, float y, float z)
{
	min.set(std::min(min.x, x), std::min(min.y, y), std::min(min.z, z));
	max.set(std::max(max.x, x), std::max(max.y, y), std::max(max.z, z));
	return set(min, max);
}

BoundingBox& BoundingBox::ext(const BoundingBox& a_bounds)
{
	min.set(std::min(min.x, a_bounds.min.x), std::min(min.y, a_bounds.min.y), std::min(min.z, a_bounds.min.z));
	max.set(std::max(max.x, a_bounds.max.x), std::max(max.y, a_bounds.max.y), std::max(max.z, a_bounds.max.z));
	return set(min, max);
}

const Vec3 BoundingBox::tmpVector;

Color3B ParticleEmitter::tempColor;

ParticleEmitter::ParticleEmitter(ParticleEmitter* emitter)
{
	init(emitter);
}

void ParticleEmitter::setPosition(float x, float y)
{
	if (!attached) {
		float xAmount = this->dx - x;
		float yAmount = this->dy - y;
		for (int i = 0; i < maxParticleCount; i++)
			if (active[i]) particles[i]->translate(xAmount, yAmount);
	}
	dx = x;
	dy = y;
}

void ParticleEmitter::translate(float x, float y)
{
	if (!attached) {
		for (int i = 0; i < maxParticleCount; i++)
			if (active[i]) particles[i]->translate(-x, -y);
	}
	dx += x;
	dy += y;
}

void ParticleEmitter::setFlip(bool flipX, bool flipY)
{
	this->_flipX = flipX;
	this->_flipY = flipY;
	updateTexCoords();
}

void ParticleEmitter::setSprite(Sprite* sprite)
{
	CC_SAFE_RELEASE_NULL(this->sprite);
	this->sprite = sprite;
	if (!sprite) return;
	sprite->retain();
	auto& size = sprite->getContentSize();
	_spriteWidth = size.width;
	_spriteHeight = size.height;
	updateTexCoords();
}

void ParticleEmitter::init(ParticleEmitter* emitter)
{
	name = emitter->name;
	imagePath = emitter->imagePath;
	setMaxParticleCount(emitter->maxParticleCount);
	minParticleCount = emitter->minParticleCount;
	delayValue.load(emitter->delayValue);
	durationValue.load(emitter->durationValue);
	emissionValue.load(emitter->emissionValue);
	lifeValue.load(emitter->lifeValue);
	lifeOffsetValue.load(emitter->lifeOffsetValue);
	scaleValue.load(emitter->scaleValue);
	rotationValue.load(emitter->rotationValue);
	velocityValue.load(emitter->velocityValue);
	angleValue.load(emitter->angleValue);
	windValue.load(emitter->windValue);
	gravityValue.load(emitter->gravityValue);
	transparencyValue.load(emitter->transparencyValue);
	tintValue.load(emitter->tintValue);
	xOffsetValue.load(emitter->xOffsetValue);
	yOffsetValue.load(emitter->yOffsetValue);
	spawnWidthValue.load(emitter->spawnWidthValue);
	spawnHeightValue.load(emitter->spawnHeightValue);
	spawnShapeValue.load(emitter->spawnShapeValue);
	attached = emitter->attached;
	continuous = emitter->continuous;
	aligned = emitter->aligned;
	behind = emitter->behind;
	additive = emitter->additive;
	premultipliedAlpha = emitter->premultipliedAlpha;
	this->_cleansUpBlendFunction = emitter->_cleansUpBlendFunction;
	setSprite(emitter->sprite);
	updateBlendFunc();
	initGLProgramState();
}

void ParticleEmitter::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
	//quad command
	if (activeCount > 0){
		_quadCommand.init(_globalZOrder, sprite->getTexture()->getName(), getGLProgramState(), _blendFunc, _quads, activeCount, transform, flags);
		renderer->addCommand(&_quadCommand);
	}
}

void ParticleEmitter::setMaxParticleCount(int maxParticleCount)
{
	if (this->maxParticleCount == maxParticleCount) return;

	if (active) delete[]active;
	active = new bool[maxParticleCount];
	for (int i = 0; i < maxParticleCount; ++i){
		active[i] = false;
	}

	if (particles){
		auto pool = ParticlePool::getInstance();
		for (int i = 0; i < this->maxParticleCount; ++i){
			pool->freeToPool(particles[i]);
		}
	}
	particles = new Particle*[maxParticleCount];
	for (int i = 0; i < maxParticleCount; ++i){
		particles[i] = nullptr;
	}


	// If we are setting the total number of particles to a number higher
	// than what is allocated, we need to allocate new arrays
	if (maxParticleCount > _allocatedParticles)
	{
		// Allocate new memory
		size_t quadsSize = sizeof(_quads[0]) * maxParticleCount * 1;
		size_t indicesSize = sizeof(_indices[0]) * maxParticleCount * 6 * 1;

		V3F_C4B_T2F_Quad* quadsNew = (V3F_C4B_T2F_Quad*)realloc(_quads, quadsSize);
		GLushort* indicesNew = (GLushort*)realloc(_indices, indicesSize);

		if (quadsNew && indicesNew){
			// Assign pointers
			_quads = quadsNew;
			_indices = indicesNew;

			// Clear the memory
			memset(_quads, 0, quadsSize);
			memset(_indices, 0, indicesSize);

			_allocatedParticles = maxParticleCount;
		}
		else{
			// Out of memory, failed to resize some array
			if (quadsNew) _quads = quadsNew;
			if (indicesNew) _indices = indicesNew;

			CCLOG("Particle system: out of memory");
			return;
		}

		this->maxParticleCount = maxParticleCount;

		initIndices();
		if (Configuration::getInstance()->supportsShareableVAO()){
			setupVBOandVAO();
		}
		else{
			setupVBO();
		}

		// fixed http://www.cocos2d-x.org/issues/3990
		// Updates texture coords.
		updateTexCoords();
	}
	else{
		this->maxParticleCount = maxParticleCount;
	}



}

void ParticleEmitter::addParticle()
{
	int activeCount = this->activeCount;
	if (activeCount == maxParticleCount) return;
	for (int i = 0; i < maxParticleCount; i++) {
		if (!active[i]) {
			activateParticle(i);
			active[i] = true;
			this->activeCount = activeCount + 1;
			break;
		}
	}
}

void ParticleEmitter::addParticles(int count)
{
	count = std::min(count, maxParticleCount - activeCount);
	if (count == 0) return;
	for (int index = 0, i = 0; i < count && index != maxParticleCount; index++) {
		if (!active[index]) {
			activateParticle(index);
			active[index] = true;
			i++;
		}
	}
	this->activeCount += count;
}

void ParticleEmitter::update(float delta)
{
	CC_PROFILER_START_CATEGORY(kProfilerCategoryParticles, "ParticleEmitter - update");
	accumulator += delta * 1000;
	if (accumulator < 1) return;
	int deltaMillis = (int)accumulator;
	accumulator -= deltaMillis;

	if (delayTimer < delay) {
		delayTimer += deltaMillis;
	}
	else {
		bool done = false;
		if (firstUpdate) {
			firstUpdate = false;
			addParticle();
		}

		if (durationTimer < duration)
			durationTimer += deltaMillis;
		else {
			if (!continuous || _allowCompletion)
				done = true;
			else
				restart();
		}

		if (!done) {
			emissionDelta += deltaMillis;
			float emissionTime = emission + emissionDiff * emissionValue.getScale(durationTimer / (float)duration);
			if (emissionTime > 0) {
				emissionTime = 1000 / emissionTime;
				if (emissionDelta >= emissionTime) {
					int emitCount = (int)(emissionDelta / emissionTime);
					emitCount = std::min(emitCount, maxParticleCount - activeCount);
					emissionDelta -= emitCount * emissionTime;
					emissionDelta = std::fmod(emissionDelta, emissionTime);
					addParticles(emitCount);
				}
			}
			if (activeCount < minParticleCount) addParticles(minParticleCount - activeCount);
		}
	}


	int activeCount = this->activeCount;
	for (int i = 0; i < maxParticleCount; i++) {
		if (active[i] && !updateParticle(particles[i], delta, deltaMillis)) {
			active[i] = false;
			activeCount--;
		}
	}
	this->activeCount = activeCount;
	updateParticleQuads();
	postStep();
	CC_PROFILER_STOP_CATEGORY(kProfilerCategoryParticles, "ParticleEmitter - update");
}

void ParticleEmitter::start()
{
	firstUpdate = true;
	_allowCompletion = false;
	restart();
}

void ParticleEmitter::reset()
{
	emissionDelta = 0;
	durationTimer = duration;
	for (int i = 0; i < maxParticleCount; i++){
		active[i] = false;
	}
	activeCount = 0;
	start();
}

void ParticleEmitter::restart()
{
	delay = delayValue.active ? delayValue.newLowValue() : 0;
	delayTimer = 0;

	durationTimer -= duration;
	duration = durationValue.newLowValue();

	emission = (int)emissionValue.newLowValue();
	emissionDiff = (int)emissionValue.newHighValue();
	if (!emissionValue.isRelative()) emissionDiff -= emission;

	life = (int)lifeValue.newLowValue();
	lifeDiff = (int)lifeValue.newHighValue();
	if (!lifeValue.isRelative()) lifeDiff -= life;

	lifeOffset = lifeOffsetValue.active ? (int)lifeOffsetValue.newLowValue() : 0;
	lifeOffsetDiff = (int)lifeOffsetValue.newHighValue();
	if (!lifeOffsetValue.isRelative()) lifeOffsetDiff -= lifeOffset;

	spawnWidth = spawnWidthValue.newLowValue();
	spawnWidthDiff = spawnWidthValue.newHighValue();
	if (!spawnWidthValue.isRelative()) spawnWidthDiff -= spawnWidth;

	spawnHeight = spawnHeightValue.newLowValue();
	spawnHeightDiff = spawnHeightValue.newHighValue();
	if (!spawnHeightValue.isRelative()) spawnHeightDiff -= spawnHeight;

	updateFlags = 0;
	if (angleValue.active && angleValue.timeline.size() > 1) updateFlags |= UPDATE_ANGLE;
	if (velocityValue.active) updateFlags |= UPDATE_VELOCITY;
	if (scaleValue.timeline.size() > 1) updateFlags |= UPDATE_SCALE;
	if (rotationValue.active && rotationValue.timeline.size() > 1) updateFlags |= UPDATE_ROTATION;
	if (windValue.active) updateFlags |= UPDATE_WIND;
	if (gravityValue.active) updateFlags |= UPDATE_GRAVITY;
	if (tintValue.timeline.size() > 1) updateFlags |= UPDATE_TINT;
}

void ParticleEmitter::initIndices()
{
	for (int i = 0; i < maxParticleCount; ++i)
	{
		const unsigned int i6 = i * 6;
		const unsigned int i4 = i * 4;
		_indices[i6 + 0] = (GLushort)i4 + 0;
		_indices[i6 + 1] = (GLushort)i4 + 1;
		_indices[i6 + 2] = (GLushort)i4 + 2;

		_indices[i6 + 5] = (GLushort)i4 + 1;
		_indices[i6 + 4] = (GLushort)i4 + 2;
		_indices[i6 + 3] = (GLushort)i4 + 3;
	}
}

// pointRect should be in Texture coordinates, not pixel coordinates
void ParticleEmitter::initTexCoordsWithRect(const Rect& pointRect){
	// convert to Tex coords

	Rect rect = Rect(
		pointRect.origin.x * CC_CONTENT_SCALE_FACTOR(),
		pointRect.origin.y * CC_CONTENT_SCALE_FACTOR(),
		pointRect.size.width * CC_CONTENT_SCALE_FACTOR(),
		pointRect.size.height * CC_CONTENT_SCALE_FACTOR());

	GLfloat wide = (GLfloat)pointRect.size.width;
	GLfloat high = (GLfloat)pointRect.size.height;

	if (sprite){
		wide = (GLfloat)sprite->getTexture()->getPixelsWide();
		high = (GLfloat)sprite->getTexture()->getPixelsHigh();
	}

#if CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL
	GLfloat left = (rect.origin.x * 2 + 1) / (wide * 2);
	GLfloat bottom = (rect.origin.y * 2 + 1) / (high * 2);
	GLfloat right = left + (rect.size.width * 2 - 2) / (wide * 2);
	GLfloat top = bottom + (rect.size.height * 2 - 2) / (high * 2);
#else
	GLfloat left = rect.origin.x / wide;
	GLfloat bottom = rect.origin.y / high;
	GLfloat right = left + rect.size.width / wide;
	GLfloat top = bottom + rect.size.height / high;
#endif // ! CC_FIX_ARTIFACTS_BY_STRECHING_TEXEL

	// Important. Texture in cocos2d are inverted, so the Y component should be inverted
	std::swap(top, bottom);

	V3F_C4B_T2F_Quad *quads = nullptr;
	unsigned int start = 0, end = 0;

	quads = _quads;
	start = 0;
	end = maxParticleCount;

	GLfloat temp;
	if (_flipX){
		temp = left;
		left = right;
		right = temp;
	}
	if (_flipY){
		temp = top;
		top = bottom;
		bottom = temp;
	}

	for (unsigned int i = start; i < end; i++)
	{
		// bottom-left vertex:
		quads[i].bl.texCoords.u = left;
		quads[i].bl.texCoords.v = bottom;
		// bottom-right vertex:
		quads[i].br.texCoords.u = right;
		quads[i].br.texCoords.v = bottom;
		// top-left vertex:
		quads[i].tl.texCoords.u = left;
		quads[i].tl.texCoords.v = top;
		// top-right vertex:
		quads[i].tr.texCoords.u = right;
		quads[i].tr.texCoords.v = top;
	}
}

void ParticleEmitter::updateTexCoords()
{
	if (sprite){
		auto& rect = sprite->getTextureRect();
		auto& s = rect.size;
		initTexCoordsWithRect(Rect(rect.getMinX(), rect.getMinY(), s.width, s.height));
	}
}

void ParticleEmitter::updateParticleQuads(){
	if (activeCount <= 0) {
		return;
	}

	V3F_C4B_T2F_Quad *startQuad = &(_quads[0]);
	for (int i = 0; i < maxParticleCount; ++i){
		if (active[i]){
			auto particle = particles[i];
			updatePosWithParticle(startQuad, particle, _spriteWidth, _spriteHeight);
			auto& color = particle->_color;
			startQuad->bl.colors.set(color.r, color.g, color.b, particle->_opacity);
			startQuad->br.colors.set(color.r, color.g, color.b, particle->_opacity);
			startQuad->tl.colors.set(color.r, color.g, color.b, particle->_opacity);
			startQuad->tr.colors.set(color.r, color.g, color.b, particle->_opacity);
			++startQuad;
		}
	}
}

void ParticleEmitter::setupVBOandVAO()
{
	// clean VAO
	glDeleteBuffers(2, &_buffersVBO[0]);
	glDeleteVertexArrays(1, &_VAOname);
	GL::bindVAO(0);

	glGenVertexArrays(1, &_VAOname);
	GL::bindVAO(_VAOname);

#define kQuadSize sizeof(_quads[0].bl)

	glGenBuffers(2, &_buffersVBO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * maxParticleCount, _quads, GL_DYNAMIC_DRAW);

	// vertices
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_POSITION);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*)offsetof(V3F_C4B_T2F, vertices));

	// colors
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_COLOR);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, kQuadSize, (GLvoid*)offsetof(V3F_C4B_T2F, colors));

	// tex coords
	glEnableVertexAttribArray(GLProgram::VERTEX_ATTRIB_TEX_COORD);
	glVertexAttribPointer(GLProgram::VERTEX_ATTRIB_TEX_COORD, 2, GL_FLOAT, GL_FALSE, kQuadSize, (GLvoid*)offsetof(V3F_C4B_T2F, texCoords));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices[0]) * maxParticleCount * 6, _indices, GL_STATIC_DRAW);

	// Must unbind the VAO before changing the element buffer.
	GL::bindVAO(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();
}

void ParticleEmitter::setupVBO()
{
	glDeleteBuffers(2, &_buffersVBO[0]);

	glGenBuffers(2, &_buffersVBO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0]) * maxParticleCount, _quads, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffersVBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices[0]) * maxParticleCount * 6, _indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();
}

void NS_CUSTOM::ParticleEmitter::postStep()
{
	glBindBuffer(GL_ARRAY_BUFFER, _buffersVBO[0]);

	// Option 1: Sub Data
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(_quads[0])*maxParticleCount, _quads);

	// Option 2: Data
	//  glBufferData(GL_ARRAY_BUFFER, sizeof(quads_[0]) * particleCount, quads_, GL_DYNAMIC_DRAW);

	// Option 3: Orphaning + glMapBuffer
	// glBufferData(GL_ARRAY_BUFFER, sizeof(_quads[0])*_totalParticles, nullptr, GL_STREAM_DRAW);
	// void *buf = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// memcpy(buf, _quads, sizeof(_quads[0])*_totalParticles);
	// glUnmapBuffer(GL_ARRAY_BUFFER);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR_DEBUG();
}

void ParticleEmitter::updateBlendFunc()
{
	if (additive){
		_blendFunc = BlendFunc::ADDITIVE;
	}
	else if (premultipliedAlpha){
		_blendFunc = BlendFunc::ALPHA_PREMULTIPLIED;
	}
	else {
		_blendFunc = BlendFunc::ALPHA_NON_PREMULTIPLIED;
	}
}

void ParticleEmitter::initGLProgramState()
{
	if (getGLProgramState()) return;
	setGLProgramState(GLProgramState::getOrCreateWithGLProgramName(GLProgram::SHADER_NAME_POSITION_TEXTURE_COLOR_NO_MVP));
}

inline void NS_CUSTOM::ParticleEmitter::updatePosWithParticle(V3F_C4B_T2F_Quad *quad, Particle* particle, float spriteW, float spriteH)
{
	// vertices
	GLfloat x2 = spriteW * particle->_scale / 2;
	GLfloat y2 = spriteH * particle->_scale / 2;

	GLfloat x1 = -x2;
	GLfloat y1 = -y2;

	GLfloat x = particle->_position.x + spriteW / 2;
	GLfloat y = particle->_position.y + spriteH / 2;

	GLfloat r = (GLfloat)CC_DEGREES_TO_RADIANS(particle->_rotation);
	GLfloat cr = cosFast(r);
	GLfloat sr = sinFast(r);

	GLfloat ax = x1 * cr - y1 * sr + x;
	GLfloat ay = x1 * sr + y1 * cr + y;
	GLfloat bx = x2 * cr - y1 * sr + x;
	GLfloat by = x2 * sr + y1 * cr + y;
	GLfloat cx = x2 * cr - y2 * sr + x;
	GLfloat cy = x2 * sr + y2 * cr + y;
	GLfloat dx = x1 * cr - y2 * sr + x;
	GLfloat dy = x1 * sr + y2 * cr + y;

	// bottom-left
	quad->bl.vertices.x = ax;
	quad->bl.vertices.y = ay;

	// bottom-right vertex:
	quad->br.vertices.x = bx;
	quad->br.vertices.y = by;

	// top-left vertex:
	quad->tl.vertices.x = dx;
	quad->tl.vertices.y = dy;

	// top-right vertex:
	quad->tr.vertices.x = cx;
	quad->tr.vertices.y = cy;
}

void ParticleEmitter::initialize()
{
	durationValue.setAlwaysActive(true);
	emissionValue.setAlwaysActive(true);
	lifeValue.setAlwaysActive(true);
	scaleValue.setAlwaysActive(true);
	transparencyValue.setAlwaysActive(true);
	spawnShapeValue.setAlwaysActive(true);
	spawnWidthValue.setAlwaysActive(true);
	spawnHeightValue.setAlwaysActive(true);
}

Particle* ParticleEmitter::newParticle(Sprite* sprite)
{
	return ParticlePool::getInstance()->getFromPool();
}

void ParticleEmitter::activateParticle(int index)
{
	if (!particles[index]) {
		particles[index] = newParticle(sprite);
	}
	auto particle = particles[index];

	float percent = durationTimer / (float)duration;
	int updateFlags = this->updateFlags;

	particle->currentLife = particle->life = life + (int)(lifeDiff * lifeValue.getScale(percent));

	if (velocityValue.active) {
		particle->velocity = velocityValue.newLowValue();
		particle->velocityDiff = velocityValue.newHighValue();
		if (!velocityValue.isRelative()) particle->velocityDiff -= particle->velocity;
	}

	particle->angle = angleValue.newLowValue();
	particle->angleDiff = angleValue.newHighValue();
	if (!angleValue.isRelative()) particle->angleDiff -= particle->angle;
	float angle = 0;
	if ((updateFlags & UPDATE_ANGLE) == 0) {
		angle = particle->angle + particle->angleDiff * angleValue.getScale(0);
		particle->angle = angle;
		particle->angleCos = cosFast(angle*M_PI / 180);
		particle->angleSin = sinFast(angle*M_PI / 180);
	}

	float spriteWidth = sprite->getContentSize().width;
	particle->scale = scaleValue.newLowValue() / spriteWidth;
	particle->scaleDiff = scaleValue.newHighValue() / spriteWidth;
	if (!scaleValue.isRelative()) particle->scaleDiff -= particle->scale;
	particle->_scale = particle->scale + particle->scaleDiff * scaleValue.getScale(0);

	if (rotationValue.active) {
		particle->rotation = rotationValue.newLowValue();
		particle->rotationDiff = rotationValue.newHighValue();
		if (!rotationValue.isRelative()) particle->rotationDiff -= particle->rotation;
		float rotation = particle->rotation + particle->rotationDiff * rotationValue.getScale(0);
		if (aligned) rotation += angle;
		particle->_rotation = rotation;
	}

	if (windValue.active) {
		particle->wind = windValue.newLowValue();
		particle->windDiff = windValue.newHighValue();
		if (!windValue.isRelative()) particle->windDiff -= particle->wind;
	}

	if (gravityValue.active) {
		particle->gravity = gravityValue.newLowValue();
		particle->gravityDiff = gravityValue.newHighValue();
		if (!gravityValue.isRelative()) particle->gravityDiff -= particle->gravity;
	}

	const float_array& temp = tintValue.getColor(0);
	particle->tint[0] = temp[0];
	particle->tint[1] = temp[1];
	particle->tint[2] = temp[2];

	particle->transparency = transparencyValue.newLowValue();
	particle->transparencyDiff = transparencyValue.newHighValue() - particle->transparency;

	// Spawn.
	float x = this->getPositionX();
	if (xOffsetValue.active) x += xOffsetValue.newLowValue();
	float y = this->getPositionY();
	if (yOffsetValue.active) y += yOffsetValue.newLowValue();
	switch (spawnShapeValue.shape) {
	case square: {
		float width = spawnWidth + (spawnWidthDiff * spawnWidthValue.getScale(percent));
		float height = spawnHeight + (spawnHeightDiff * spawnHeightValue.getScale(percent));
		x += randomf(0.0f, width) - width / 2;
		y += randomf(0.0f, height) - height / 2;
		break;
	}
	case ellipse: {
		float width = spawnWidth + (spawnWidthDiff * spawnWidthValue.getScale(percent));
		float height = spawnHeight + (spawnHeightDiff * spawnHeightValue.getScale(percent));
		float radiusX = width / 2;
		float radiusY = height / 2;
		if (radiusX == 0 || radiusY == 0) break;
		float scaleY = radiusX / (float)radiusY;
		if (spawnShapeValue.edges) {
			float spawnAngle;
			switch (spawnShapeValue.side) {
			case top:
				spawnAngle = -random(0.0f, 179.0f);
				break;
			case bottom:
				spawnAngle = random(0.0f, 179.0f);
				break;
			default:
				spawnAngle = random(0.0f, 360.0f);
				break;
			}
			float cosDeg = cosFast(spawnAngle*M_PI / 180);
			float sinDeg = sinFast(spawnAngle*M_PI / 180);
			x += cosDeg * radiusX;
			y += sinDeg * radiusX / scaleY;
			if ((updateFlags & UPDATE_ANGLE) == 0) {
				particle->angle = spawnAngle;
				particle->angleCos = cosDeg;
				particle->angleSin = sinDeg;
			}
		}
		else {
			float radius2 = radiusX * radiusX;
			while (true) {
				float px = randomf(0.0f, width) - radiusX;
				float py = randomf(0.0f, height) - radiusY;
				if (px * px + py * py <= radius2) {
					x += px;
					y += py / scaleY;
					break;
				}
			}
		}
		break;
	}
	case line: {
		float width = spawnWidth + (spawnWidthDiff * spawnWidthValue.getScale(percent));
		float height = spawnHeight + (spawnHeightDiff * spawnHeightValue.getScale(percent));
		if (width != 0) {
			float lineX = randomf(0.0f, width);
			x += lineX;
			y += lineX * (height / (float)width);
		}
		else
			y += randomf(0.0f, height);
		break;
	}
	}

	float spriteHeight = sprite->getContentSize().height;
	particle->setPosition(x - spriteWidth / 2, y - spriteHeight / 2);

	int offsetTime = (int)(lifeOffset + lifeOffsetDiff * lifeOffsetValue.getScale(percent));
	if (offsetTime > 0) {
		if (offsetTime >= particle->currentLife) offsetTime = particle->currentLife - 1;
		updateParticle(particle, offsetTime / 1000.0f, offsetTime);
	}
}

bool ParticleEmitter::updateParticle(Particle* particle, float delta, int deltaMillis)
{
	int life = particle->currentLife - deltaMillis;
	if (life <= 0) return false;
	particle->currentLife = life;

	float percent = 1 - particle->currentLife / (float)particle->life;
	int updateFlags = this->updateFlags;

	if ((updateFlags & UPDATE_SCALE) != 0)
		particle->_scale = particle->scale + particle->scaleDiff * scaleValue.getScale(percent);

	if ((updateFlags & UPDATE_VELOCITY) != 0) {
		float velocity = (particle->velocity + particle->velocityDiff * velocityValue.getScale(percent)) * delta;

		float velocityX, velocityY;
		if ((updateFlags & UPDATE_ANGLE) != 0) {
			float angle = particle->angle + particle->angleDiff * angleValue.getScale(percent);
			velocityX = velocity * cosFast(angle*M_PI / 180);
			velocityY = velocity * sinFast(angle*M_PI / 180);
			if ((updateFlags & UPDATE_ROTATION) != 0) {
				float rotation = particle->rotation + particle->rotationDiff * rotationValue.getScale(percent);
				if (aligned) rotation += angle;
				particle->_rotation = rotation;
			}
		}
		else {
			velocityX = velocity * particle->angleCos;
			velocityY = velocity * particle->angleSin;
			if (aligned || (updateFlags & UPDATE_ROTATION) != 0) {
				float rotation = particle->rotation + particle->rotationDiff * rotationValue.getScale(percent);
				if (aligned) rotation += particle->angle;
				particle->_rotation = rotation;
			}
		}

		if ((updateFlags & UPDATE_WIND) != 0)
			velocityX += (particle->wind + particle->windDiff * windValue.getScale(percent)) * delta;

		if ((updateFlags & UPDATE_GRAVITY) != 0)
			velocityY += (particle->gravity + particle->gravityDiff * gravityValue.getScale(percent)) * delta;

		particle->translate(velocityX, velocityY);
	}
	else {
		if ((updateFlags & UPDATE_ROTATION) != 0)
			particle->_rotation = particle->rotation + particle->rotationDiff * rotationValue.getScale(percent);
	}

	float red, green, blue;
	if ((updateFlags & UPDATE_TINT) != 0){
		const auto& color = tintValue.getColor(percent);
		red = color[0];
		green = color[1];
		blue = color[2];
	}
	else{
		red = particle->tint[0];
		green = particle->tint[1];
		blue = particle->tint[2];
	}

	if (premultipliedAlpha) {
		float alphaMultiplier = additive ? 0 : 1;
		float a = particle->transparency + particle->transparencyDiff * transparencyValue.getScale(percent);
		tempColor.r = red * a * 255;
		tempColor.g = green * a * 255;
		tempColor.b = blue * a * 255;
		particle->_color = tempColor;
		particle->_opacity = a * alphaMultiplier * 255;
	}
	else {
		tempColor.r = red * 255;
		tempColor.g = green * 255;
		tempColor.b = blue * 255;
		particle->_color = tempColor;
		particle->_opacity = particle->transparency + particle->transparencyDiff * transparencyValue.getScale(percent) * 255;
	}
	return true;
}

bool ParticleEmitter::isComplete()
{
	if (continuous) return false;
	if (delayTimer < delay) return false;
	return durationTimer >= duration && activeCount == 0;
}

float ParticleEmitter::getPercentComplete()
{
	if (delayTimer < delay) return 0;
	return std::min(1.0f, durationTimer / (float)duration);
}

void ParticleEmitter::flipY()
{
	angleValue.setHigh(-angleValue.getHighMin(), -angleValue.getHighMax());
	angleValue.setLow(-angleValue.getLowMin(), -angleValue.getLowMax());

	gravityValue.setHigh(-gravityValue.getHighMin(), -gravityValue.getHighMax());
	gravityValue.setLow(-gravityValue.getLowMin(), -gravityValue.getLowMax());

	windValue.setHigh(-windValue.getHighMin(), -windValue.getHighMax());
	windValue.setLow(-windValue.getLowMin(), -windValue.getLowMax());

	rotationValue.setHigh(-rotationValue.getHighMin(), -rotationValue.getHighMax());
	rotationValue.setLow(-rotationValue.getLowMin(), -rotationValue.getLowMax());

	yOffsetValue.setLow(-yOffsetValue.getLowMin(), -yOffsetValue.getLowMax());
}

ostream& ParticleEmitter::save(ostream& output)
{
	output << name << "\n";
	output << "- Delay -\n";
	delayValue.save(output);
	output << "- Duration - \n";
	durationValue.save(output);
	output << "- Count - \n";
	output << "min: " << minParticleCount << "\n";
	output << "max: " << maxParticleCount << "\n";
	output << "- Emission - \n";
	emissionValue.save(output);
	output << "- Life - \n";
	lifeValue.save(output);
	output << "- Life Offset - \n";
	lifeOffsetValue.save(output);
	output << "- X Offset - \n";
	xOffsetValue.save(output);
	output << "- Y Offset - \n";
	yOffsetValue.save(output);
	output << "- Spawn Shape - \n";
	spawnShapeValue.save(output);
	output << "- Spawn Width - \n";
	spawnWidthValue.save(output);
	output << "- Spawn Height - \n";
	spawnHeightValue.save(output);
	output << "- Scale - \n";
	scaleValue.save(output);
	output << "- Velocity - \n";
	velocityValue.save(output);
	output << "- Angle - \n";
	angleValue.save(output);
	output << "- Rotation - \n";
	rotationValue.save(output);
	output << "- Wind - \n";
	windValue.save(output);
	output << "- Gravity - \n";
	gravityValue.save(output);
	output << "- Tint - \n";
	tintValue.save(output);
	output << "- Transparency - \n";
	transparencyValue.save(output);
	output << "- Options - \n";
	output << "attached: " << (attached ? "true" : "false") << "\n";
	output << "continuous: " << (continuous ? "true" : "false") << "\n";
	output << "aligned: " << (aligned ? "true" : "false") << "\n";
	output << "additive: " << (additive ? "true" : "false") << "\n";
	output << "behind: " << (behind ? "true" : "false") << "\n";
	output << "premultipliedAlpha: " << (premultipliedAlpha ? "true" : "false") << "\n";
	output << "- Image Path -\n";
	output << imagePath << "\n";
	return output;
}

void ParticleEmitter::load(istream& reader)
{
	string line;
	name = readString(reader, "name");
	getline(reader, line);
	delayValue.load(reader);
	getline(reader, line);
	durationValue.load(reader);
	getline(reader, line);
	setMinParticleCount(readInt(reader, "minParticleCount"));
	setMaxParticleCount(readInt(reader, "maxParticleCount"));
	getline(reader, line);
	emissionValue.load(reader);
	getline(reader, line);
	lifeValue.load(reader);
	getline(reader, line);
	lifeOffsetValue.load(reader);
	getline(reader, line);
	xOffsetValue.load(reader);
	getline(reader, line);
	yOffsetValue.load(reader);
	getline(reader, line);
	spawnShapeValue.load(reader);
	getline(reader, line);
	spawnWidthValue.load(reader);
	getline(reader, line);
	spawnHeightValue.load(reader);
	getline(reader, line);
	scaleValue.load(reader);
	getline(reader, line);
	velocityValue.load(reader);
	getline(reader, line);
	angleValue.load(reader);
	getline(reader, line);
	rotationValue.load(reader);
	getline(reader, line);
	windValue.load(reader);
	getline(reader, line);
	gravityValue.load(reader);
	getline(reader, line);
	tintValue.load(reader);
	getline(reader, line);
	transparencyValue.load(reader);
	getline(reader, line);
	attached = readBoolean(reader, "attached");
	continuous = readBoolean(reader, "continuous");
	aligned = readBoolean(reader, "aligned");
	additive = readBoolean(reader, "additive");
	behind = readBoolean(reader, "behind");

	// Backwards compatibility
	getline(reader, line);
	if (startWith(line, "premultipliedAlpha")) {
		premultipliedAlpha = readBoolean(line);
		getline(reader, line);
	}
	getline(reader, line);
	setImagePath(trim(line));

	updateBlendFunc();
	initGLProgramState();
}

ostream& NumericValue::save(ostream& output)
{
	ParticleValue::save(output);
	if (!active) return output;
	output << "value: " << value << "\n";
	return  output;
}

void NumericValue::load(NumericValue& value)
{
	ParticleValue::load(value);
	this->value = value.value;
}

void NumericValue::load(istream& reader)
{
	ParticleValue::load(reader);
	if (!active) return;
	value = readFloat(reader, "value");
}

ostream& RangedNumericValue::save(ostream& output)
{
	ParticleValue::save(output);
	if (!active) return output;
	output << "lowMin: " << lowMin << "\n";
	output << "lowMax: " << lowMax << "\n";
	return output;
}

void NS_CUSTOM::RangedNumericValue::load(RangedNumericValue& value)
{
	ParticleValue::load(value);
	lowMax = value.lowMax;
	lowMin = value.lowMin;
}

void NS_CUSTOM::RangedNumericValue::load(istream& reader)
{
	ParticleValue::load(reader);
	if (!active) return;
	lowMin = readFloat(reader, "lowMin");
	lowMax = readFloat(reader, "lowMax");
}

float ScaledNumericValue::getScale(float percent)
{
	int endIndex = -1;
	int n = timeline.size();
	for (int i = 1; i < n; i++) {
		float t = timeline[i];
		if (t > percent) {
			endIndex = i;
			break;
		}
	}
	if (endIndex == -1) return scaling[n - 1];
	int startIndex = endIndex - 1;
	float startValue = scaling[startIndex];
	float startTime = timeline[startIndex];
	return startValue + (scaling[endIndex] - startValue) * ((percent - startTime) / (timeline[endIndex] - startTime));

}

ostream& ScaledNumericValue::save(ostream& output)
{
	RangedNumericValue::save(output);
	if (!active) return output;
	output << "highMin: " << highMin << "\n";
	output << "highMax: " << highMax << "\n";
	output << "relative: " << (relative ? "true" : "false") << "\n";
	output << "scalingCount: " << scaling.size() << "\n";
	for (int i = 0; i != scaling.size(); i++)
		output << "scaling" << i << ": " << scaling[i] << "\n";
	output << "timelineCount: " << timeline.size() << "\n";
	for (int i = 0; i != timeline.size(); i++)
		output << "timeline" << i << ": " << timeline[i] << "\n";
	return output;
}

void ScaledNumericValue::load(ScaledNumericValue& value)
{
	RangedNumericValue::load(value);
	highMax = value.highMax;
	highMin = value.highMin;
	scaling.clear();
	for (float v : value.scaling){
		scaling.push_back(v);
	}
	timeline.clear();
	for (float v : value.timeline){
		timeline.push_back(v);
	}
	relative = value.relative;
}

void ScaledNumericValue::load(istream& reader)
{
	RangedNumericValue::load(reader);
	if (!active) return;
	highMin = readFloat(reader, "highMin");
	highMax = readFloat(reader, "highMax");
	relative = readBoolean(reader, "relative");
	scaling.clear();
	int scalingCount = readInt(reader, "scalingCount");
	for (int i = 0; i < scalingCount; i++){
		scaling.push_back(readFloat(reader, "scaling" + i));
	}
	timeline.clear();
	int timelineCount = readInt(reader, "timelineCount");
	for (int i = 0; i < timelineCount; i++){
		timeline.push_back(readFloat(reader, "timeline" + i));
	}
}

ostream& SpawnShapeValue::save(ostream& output)
{
	ParticleValue::save(output);
	if (!active) return output;
	for (auto& pair : SpawnShapeMap){
		if (pair.second == shape){
			output << "shape: " << pair.first << "\n";
			break;
		}
	}
	if (shape == SpawnShape::ellipse) {
		output << "edges: " << (edges ? "true" : "false") << "\n";
		for (auto& pair : SpawnEllipseSideMap){
			if (pair.second == side){
				output << "side: " << pair.first << "\n";
				break;
			}
		}
	}
	return output;
}

void SpawnShapeValue::load(SpawnShapeValue& value)
{
	ParticleValue::load(value);
	shape = value.shape;
	edges = value.edges;
	side = value.side;
}

void SpawnShapeValue::load(istream& reader)
{
	ParticleValue::load(reader);
	if (!active) return;
	shape = SpawnShapeMap[readString(reader, "shape")];
	if (shape == SpawnShape::ellipse) {
		edges = readBoolean(reader, "edges");
		side = SpawnEllipseSideMap[readString(reader, "side")];
	}
}

ostream& ParticleValue::save(ostream& output)
{
	if (!alwaysActive)
		output << "active: " << (active ? "true" : "false") << "\n";
	else
		active = true;
	return output;
}

void NS_CUSTOM::ParticleValue::load(istream& reader)
{
	if (!alwaysActive)
		active = readBoolean(reader, "active");
	else
		active = true;
}

void NS_CUSTOM::ParticleValue::load(ParticleValue& value)
{
	active = value.active;
	alwaysActive = value.alwaysActive;
}

string NS_CUSTOM::readString(string line)
{
	return trim(line.substr(line.find(":") + 1));
}

string NS_CUSTOM::readString(istream& reader, string name)
{
	string line;
	getline(reader, line);
	if (line.empty()) return line;
	return readString(line);
}

bool NS_CUSTOM::readBoolean(string line)
{
	return Value(readString(line)).asBool();
}

bool NS_CUSTOM::readBoolean(istream& reader, string name)
{
	return Value(readString(reader, name)).asBool();
}

int NS_CUSTOM::readInt(istream& reader, string name)
{
	return Value(readString(reader, name)).asInt();
}

float NS_CUSTOM::readFloat(istream& reader, string name)
{
	return Value(readString(reader, name)).asFloat();
}

float NS_CUSTOM::randomf(float a, float b)
{
	return a < b ? random(a, b) : random(b, a);
}

ParticlePool* NS_CUSTOM::ParticlePool::_instance = nullptr;

NS_CUSTOM::ParticlePool::~ParticlePool()
{
	clearPool();
	if (this == _instance) _instance = nullptr;
}

void NS_CUSTOM::ParticlePool::freeToPool(Particle* particle)
{
	if (!particle) return;
	if (_size < _capacity){
		*(_pool + _size) = particle;
		_size++;
	}
	else{
		delete particle;
	}
}

Particle* NS_CUSTOM::ParticlePool::getFromPool()
{
	if (_size > 0){
		_size--;
		auto pos = _pool + _size;
		auto particle = *pos;
		*pos = nullptr;
		return particle;
		_size--;
	}
	else{
		return new Particle();
	}
}

void NS_CUSTOM::ParticlePool::init(int capacity)
{
	clearPool();
	if (capacity <= 0){
		_capacity = _size = 0;
		return;
	}
	_pool = new Particle*[capacity];
	_capacity = capacity;
	_size = 0;
}

ParticlePool* NS_CUSTOM::ParticlePool::getInstance()
{
	if (!_instance){
		_instance = new ParticlePool();
	}
	return _instance;
}

void NS_CUSTOM::ParticlePool::clearPool()
{
	if (_pool){
		auto particle = *_pool;
		for (int i = 0; i < _size; ++i, ++particle){
			delete particle;
		}
		delete[]_pool;
		_pool = nullptr;
	}
}


