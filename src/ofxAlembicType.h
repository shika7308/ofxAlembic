#pragma once

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreOgawa/All.h>

#include "ofMain.h"

#include "ofxAlembicUtil.h"

namespace ofxAlembic
{
class PolyMesh;
class Points;
class Curves;
class Camera;
class XForm;

struct Point;

enum Type
{
	POINTS = 0,
	CURVES,
	POLYMESH,
	CAMERA,
	XFORM,
	UNKHOWN
};
}

class ofxAlembic::XForm
{
public:
	Imath::M44f mat;
	
	XForm() {}
	XForm(const glm::mat4& matrix);
	
	void draw();
	
	void get(Alembic::AbcGeom::OXformSchema &schema) const;
	int64_t set(Alembic::AbcGeom::IXformSchema &schema, double time);
	double set(Alembic::AbcGeom::IXformSchema& schema, int64_t frame);
};

class ofxAlembic::PolyMesh
{
public:
	ofMesh mesh;

	PolyMesh() {}
	PolyMesh(const ofMesh& mesh) : mesh(mesh) {}

	void get(Alembic::AbcGeom::OPolyMeshSchema &schema) const;
	int64_t set(Alembic::AbcGeom::IPolyMeshSchema& schema, double time);
	double set(Alembic::AbcGeom::IPolyMeshSchema& schema, int64_t frame);

	void draw();
};

struct ofxAlembic::Point
{
	uint64_t id;
	glm::vec3 pos;
	
	Point() : id(-1) {}
	Point(const glm::vec3& pos) : id(-1), pos(pos) {}
	Point(uint64_t id, const glm::vec3& pos) : id(id), pos(pos) {}
	
	Point(float x, float y, float z) : id(-1), pos(x, y, z) {}
	Point(uint64_t id, float x, float y, float z) : id(id), pos(x, y, z) {}
};

class ofxAlembic::Points
{
public:
	vector<Point> points;
	
	Points() {}
	Points(const vector<glm::vec3>& points);
	Points(const vector<Point>& points) : points(points) {}

	void get(Alembic::AbcGeom::OPointsSchema &schema) const;
	int64_t set(Alembic::AbcGeom::IPointsSchema &schema, double time);
	double set(Alembic::AbcGeom::IPointsSchema& schema, int64_t frame);

	void draw();
};

class ofxAlembic::Curves
{
public:
	vector<ofPolyline> curves;

	Curves() {}
	Curves(const vector<ofPolyline> &curves) : curves(curves) {}

	void get(Alembic::AbcGeom::OCurvesSchema &schema) const;
	int64_t set(Alembic::AbcGeom::ICurvesSchema& schema, double time);
	double set(Alembic::AbcGeom::ICurvesSchema& schema, int64_t frame);

	void draw();
};

class ofxAlembic::Camera
{
public:

	Camera() : width(0), height(0) {}
	Camera(const ofCamera& camera) : width(0), height(0) {}
	
	void get(Alembic::AbcGeom::OCameraSchema &schema) const;
	int64_t set(Alembic::AbcGeom::ICameraSchema& schema, double time);
	double set(Alembic::AbcGeom::ICameraSchema& schema, int64_t frame);
	
	void setViewport(int width, int height) { this->width = width, this->height = height; }
	
	void updateParams(ofCamera &camera, ofMatrix4x4 xform);
	void updateSample(const ofCamera &camera);
	
	void draw();
	
protected:
	
	int width, height;
	Alembic::AbcGeom::CameraSample sample;
};