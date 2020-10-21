#include "ofxAlembicReader.h"

using namespace ofxAlembic;
using namespace Alembic::AbcGeom;

#pragma mark - IXform

ofxAlembic::IXform::IXform(Alembic::AbcGeom::IXform object) : ofxAlembic::IGeom(object), m_xform(object)
{
	update_timestamp(m_xform);
	type = XFORM;
	
	if (m_xform.getSchema().isConstant())
	{
		xform.set(m_xform.getSchema(), m_minTime);
	}
	else
	{
		xform.mat.setScale(0);
	}
}

ofxAlembic::IXform::~IXform()
{
	if (m_xform)
		m_xform.reset();
}

frame_t ofxAlembic::IXform::updateWithTimeInternal(double time, Imath::M44f& xform)
{
	frame_t ret = 0;
	if (!m_xform.getSchema().isConstant()
		&& ofInRange(time, m_minTime, m_maxTime))
	{
		ret = this->xform.set(m_xform.getSchema(), time);
	}
	
	xform = this->xform.mat * xform;

	return ret;
}

double ofxAlembic::IXform::updateWithFrameInternal(frame_t frame, Imath::M44f& xform)
{
	double ret = 0;
	if (!m_xform.getSchema().isConstant()
		&& 0 <= frame && frame < m_frameCount)
	{
		ret = this->xform.set(m_xform.getSchema(), (int64_t)frame);
	}

	xform = this->xform.mat * xform;

	return ret;
}

#pragma mark - IPoints

ofxAlembic::IPoints::IPoints(Alembic::AbcGeom::IPoints object) : ofxAlembic::IGeom(object), m_points(object)
{
	update_timestamp(m_points);
	type = POINTS;
	
	if (m_points.getSchema().isConstant())
	{
		points.set(m_points.getSchema(), m_minTime);
	}
}

frame_t ofxAlembic::IPoints::updateWithTimeInternal(double time, Imath::M44f& xform)
{
	if (m_points.getSchema().isConstant()) return 0;
	auto frame = points.set(m_points.getSchema(), time);
	return frame;
}

double ofxAlembic::IPoints::updateWithFrameInternal(frame_t frame, Imath::M44f& xform)
{
	if (m_points.getSchema().isConstant()) return 0;
	auto time = points.set(m_points.getSchema(), (int64_t)frame);
	return time;
}

#pragma mark - ICurves

ofxAlembic::ICurves::ICurves(Alembic::AbcGeom::ICurves object) : ofxAlembic::IGeom(object), m_curves(object)
{
	update_timestamp(m_curves);
	type = CURVES;
	
	if (m_curves.getSchema().isConstant())
	{
		curves.set(m_curves.getSchema(), m_minTime);
	}
}

frame_t ofxAlembic::ICurves::updateWithTimeInternal(double time, Imath::M44f& xform)
{
	if (m_curves.getSchema().isConstant()) return 0;
	auto frame = curves.set(m_curves.getSchema(), time);
	return frame;
}

double ofxAlembic::ICurves::updateWithFrameInternal(frame_t frame, Imath::M44f& xform)
{
	if (m_curves.getSchema().isConstant()) return 0;
	auto time = curves.set(m_curves.getSchema(), (int64_t)frame);
	return time;
}

#pragma mark - IPolyMesh

ofxAlembic::IPolyMesh::IPolyMesh(Alembic::AbcGeom::IPolyMesh object) : ofxAlembic::IGeom(object), m_polyMesh(object)
{
	update_timestamp(m_polyMesh);
	type = POLYMESH;
	
	if (m_polyMesh.getSchema().isConstant())
	{
		polymesh.set(m_polyMesh.getSchema(), m_minTime);
	}
}

frame_t ofxAlembic::IPolyMesh::updateWithTimeInternal(double time, Imath::M44f& xform)
{
	if (m_polyMesh.getSchema().isConstant()) return 0;
	auto frame = polymesh.set(m_polyMesh.getSchema(), time);
	return frame;
}

double ofxAlembic::IPolyMesh::updateWithFrameInternal(frame_t frame, Imath::M44f& xform)
{
	if (m_polyMesh.getSchema().isConstant()) return 0;
	auto time = polymesh.set(m_polyMesh.getSchema(), (int64_t)frame);
	return time;
}

#pragma mark - ICamera

ofxAlembic::ICamera::ICamera(Alembic::AbcGeom::ICamera object) : ofxAlembic::IGeom(object), m_camera(object)
{
	update_timestamp(m_camera);
	type = CAMERA;
	
	if (m_camera.getSchema().isConstant())
	{
		camera.set(m_camera.getSchema(), m_minTime);
	}
}

frame_t ofxAlembic::ICamera::updateWithTimeInternal(double time, Imath::M44f& xform)
{
	if (m_camera.getSchema().isConstant()) return 0;
	auto frame = camera.set(m_camera.getSchema(), time);
	return frame;
}

double ofxAlembic::ICamera::updateWithFrameInternal(frame_t frame, Imath::M44f& xform)
{
	if (m_camera.getSchema().isConstant()) return 0;
	auto time = camera.set(m_camera.getSchema(), (int64_t)frame);
	return time;
}

#pragma mark - Reader

bool ofxAlembic::Reader::open(const string& path)
{
	ofxAlembic::init();
	
	if (!ofFile::doesFileExist(path))
	{
		ofLogError("ofxAlembic") << "file not found: " << path;
		return false;
	}

	m_archive = IArchive(Alembic::AbcCoreOgawa::ReadArchive(), ofToDataPath(path),
		Alembic::Abc::ErrorHandler::kQuietNoopPolicy);
	if (!m_archive.valid()) {
		m_archive = IArchive(Alembic::AbcCoreOgawa::ReadArchive(), ofToDataPath(path),
			Alembic::Abc::ErrorHandler::kNoisyNoopPolicy);
		if (!m_archive.valid()) return false;
	}

	m_root = ofPtr<IGeom>(new IGeom(m_archive.getTop()));

	{
		object_arr.clear();
		object_name_arr.clear();
		object_fullname_arr.clear();
		object_name_map.clear();

		ofxAlembic::IGeom::visit_geoms(m_root, object_name_map, object_fullname_map);

		{
			map<string, IGeom*>::iterator it = object_name_map.begin();
			while (it != object_name_map.end())
			{
				object_arr.push_back(it->second);
				object_name_arr.push_back(it->first);
				it++;
			}
		}
		
		{
			map<string, IGeom*>::iterator it = object_fullname_map.begin();
			while (it != object_fullname_map.end())
			{
				object_fullname_arr.push_back(it->first);
				it++;
			}
		}
	}

	m_minTime = m_root->m_minTime;
	m_maxTime = m_root->m_maxTime;
	m_frameCount = m_root->m_frameCount;

	return true;
}

void ofxAlembic::Reader::close()
{
	object_arr.clear();
	object_name_arr.clear();
	object_fullname_arr.clear();
	object_name_map.clear();

	if (m_root)
		m_root.reset();

	if (m_archive.valid())
		m_archive.reset();
}

void ofxAlembic::Reader::draw()
{
	if (!m_root) return;

	m_root->draw();
}

void ofxAlembic::Reader::debugDraw()
{
	if (!m_root) return;
	
	m_root->debugDraw();
}

void ofxAlembic::Reader::setTime(double time)
{
	if (!m_root) return;

	Imath::M44f m;
	m.makeIdentity();
	auto frame = m_root->updateWithTime(time, m);

	current_time = time;
	current_frame = frame;
}

void ofxAlembic::Reader::setFrame(frame_t frame)
{
	if (!m_root) return;

	Imath::M44f m;
	m.makeIdentity();
	auto time = m_root->updateWithFrame(frame, m);

	current_frame = frame;
	current_time = time;
}

void ofxAlembic::Reader::dumpNames()
{
	const vector<string> &names = getNames();

	for (int i = 0; i < names.size(); i++)
	{
		cout << i << ": " << object_name_map[names[i]]->getTypeName() << " '" << names[i] << "'" << endl;
	}
}

void ofxAlembic::Reader::dumpFullnames()
{
	const vector<string> &names = getFullnames();
	
	for (int i = 0; i < names.size(); i++)
	{
		cout << i << ": " << object_fullname_map[names[i]]->getTypeName() << " '" << names[i] << "'" << endl;
	}
}

bool ofxAlembic::Reader::get(const string& path, ofMatrix4x4& matrix)
{
	IGeom *o = get(path);
	if (o == NULL) return false;
	return o->get(matrix);
}

bool ofxAlembic::Reader::get(const string& path, ofMesh& mesh)
{
	IGeom *o = get(path);
	if (o == NULL) return false;
	return o->get(mesh);
}

bool ofxAlembic::Reader::get(const string& path, vector<ofPolyline>& curves)
{
	IGeom *o = get(path);
	if (o == NULL) return false;
	return o->get(curves);
}

bool ofxAlembic::Reader::get(const string& path, vector<glm::vec3>& points)
{
	IGeom *o = get(path);
	if (o == NULL) return false;
	return o->get(points);
}

bool ofxAlembic::Reader::get(const string& path, ofCamera &camera)
{
	IGeom *o = get(path);
	if (o == NULL) return false;
	return o->get(camera);
}

bool ofxAlembic::Reader::get(size_t idx, ofMatrix4x4& matrix)
{
	IGeom *o = get(idx);
	if (o == NULL) return false;
	return o->get(matrix);
}

bool ofxAlembic::Reader::get(size_t idx, ofMesh& mesh)
{
	IGeom *o = get(idx);
	if (o == NULL) return false;
	return o->get(mesh);
}

bool ofxAlembic::Reader::get(size_t idx, vector<ofPolyline>& curves)
{
	IGeom *o = get(idx);
	if (o == NULL) return false;
	return o->get(curves);
}

bool ofxAlembic::Reader::get(size_t idx, vector<ofVec3f>& points)
{
	IGeom *o = get(idx);
	if (o == NULL) return false;
	return o->get(points);
}

bool ofxAlembic::Reader::get(size_t idx, ofCamera &camera)
{
	IGeom *o = get(idx);
	if (o == NULL) return false;
	return o->get(camera);
}

bool ofxAlembic::Reader::get(ofCamera& camera)
{
	size_t i = 0;
	for each (auto & obj in object_arr)
	{
		if (obj->isTypeOf<ofxAlembic::Camera>())
			return this->get(i, camera);
		i++;
	}
	return false;
}

#pragma mark - IGeom

IGeom::IGeom() : m_minTime(std::numeric_limits<float>::infinity()), m_maxTime(0), type(UNKHOWN), m_frameCount(0), index(0) {}

IGeom::IGeom(Alembic::AbcGeom::IObject object) : m_object(object), m_minTime(std::numeric_limits<float>::infinity()), m_maxTime(0), type(UNKHOWN)
{
	setupWithObject(m_object);
}

IGeom::~IGeom()
{
	m_children.clear();

	if (m_object)
		m_object.reset();
}

template <class ISchema>
frame_t getFrameCount(Alembic::AbcGeom::ISchemaObject<ISchema> obj) {
	return obj.getSchema().getNumSamples();
}

template <class TMatcher, class TMapped>
bool initializePtr(ofPtr<IGeom>& dptr, frame_t& frameCount, IObject& object, const ObjectHeader& ohead)
{
	if (TMatcher::matches(ohead))
	{
		TMatcher obj(object, ohead.getName());
		if (obj)
		{
			dptr.reset(new TMapped(obj));
			frameCount = getFrameCount(obj);
		}
		return true;
	}
	return false;
}

void IGeom::setupWithObject(IObject object)
{
	size_t numChildren = object.getNumChildren();
	
	for (size_t i = 0; i < numChildren; ++i)
	{
		const ObjectHeader &ohead = object.getChildHeader(i);

		auto test = [&] { return i + 1; };
		auto aaa = test();

		ofPtr<IGeom> dptr;
		frame_t frameCount;
		if (initializePtr< Alembic::AbcGeom::IPolyMesh, ofxAlembic::IPolyMesh>(dptr, frameCount, object, ohead)) {}
		else if (initializePtr< Alembic::AbcGeom::IPoints, ofxAlembic::IPoints>(dptr, frameCount, object, ohead)) {}
		else if (initializePtr< Alembic::AbcGeom::ICurves, ofxAlembic::ICurves>(dptr, frameCount, object, ohead)) {}
		else if (initializePtr< Alembic::AbcGeom::IXform, ofxAlembic::IXform>(dptr, frameCount, object, ohead)) {}
		else if (initializePtr< Alembic::AbcGeom::ICamera, ofxAlembic::ICamera>(dptr, frameCount, object, ohead)) {}
		else if (Alembic::AbcGeom::INuPatch::matches(ohead))
		{
			ofLogError("ofxAlembic") << "INuPatch not implemented";
			assert(false);

			//			Alembic::AbcGeom::INuPatch nuPatch(object, ohead.getName());
			//			if ( nuPatch )
			//			{
			//				dptr.reset( new INuPatchDrw( nuPatch ) );
			//			}
		}
		else if (Alembic::AbcGeom::ISubD::matches(ohead))
		{
			ofLogError("ofxAlembic") << "ISubD not implemented";
			assert(false);

			//			Alembic::AbcGeom::ISubD subd(object, ohead.getName());
			//			if ( subd )
			//			{
			//				dptr.reset( new ISubDDrw( subd ) );
			//			}
		}
		else
		{
			ofLogError("ofxAlembic") << "unknown object type: " << ohead.getFullName();
		}

		if (dptr && dptr->valid())
		{
			dptr->index = m_children.size();
			m_children.push_back(dptr);
			m_minTime = std::min(m_minTime, dptr->m_minTime);
			m_maxTime = std::max(m_maxTime, dptr->m_maxTime);
			m_frameCount = frameCount;
		}
	}
}

void IGeom::draw()
{
	ofPushMatrix();
	ofMultMatrix(transform);
	drawInternal();
	ofPopMatrix();

	for (int i = 0; i < m_children.size(); i++)
	{
		ofPtr<IGeom> c = m_children[i];
		c->draw();
	}
}

void IGeom::draw(const std::function<void( IGeom& )>& predraw)
{
	ofPushMatrix();
	ofMultMatrix(transform);
	predraw(*this);
	drawInternal();
	ofPopMatrix();

	for (int i = 0; i < m_children.size(); i++)
	{
		ofPtr<IGeom> c = m_children[i];
		c->draw(predraw);
	}
}

void IGeom::debugDraw()
{
	ofPushMatrix();
	ofMultMatrix(transform);
	debugDrawInternal();
	ofPopMatrix();
	
	for (int i = 0; i < m_children.size(); i++)
	{
		ofPtr<IGeom> c = m_children[i];
		c->debugDraw();
	}
}

string IGeom::getName() const
{
	return m_object.getName();
}

string IGeom::getFullName() const
{
	return m_object.getFullName();
}

frame_t IGeom::updateWithTime(double time, Imath::M44f& xform)
{
	auto ret = updateWithTimeInternal(time, xform);
	transform = toOf(xform);
	
	for (int i = 0; i < m_children.size(); i++)
	{
		Imath::M44f m = xform;
		auto childFrame = m_children[i]->updateWithTime(time, m);
		if (ret < childFrame)
			ret = childFrame;
	}

	return ret;
}

double IGeom::updateWithFrame(frame_t frame, Imath::M44f& xform)
{
	auto ret = updateWithFrameInternal(frame, xform);
	transform = toOf(xform);

	for (int i = 0; i < m_children.size(); i++)
	{
		Imath::M44f m = xform;
		auto childTime = m_children[i]->updateWithFrame(frame, m);
		if (ret < childTime)
			ret = childTime;
	}

	return ret;
}

template <typename T>
void ofxAlembic::IGeom::update_timestamp(T& object)
{
	TimeSamplingPtr iTsmp = object.getSchema().getTimeSampling();
	if (!object.getSchema().isConstant())
	{
		size_t numSamps =  object.getSchema().getNumSamples();
		if (numSamps > 0)
		{
			m_minTime = iTsmp->getSampleTime(0);
			m_maxTime = iTsmp->getSampleTime(numSamps - 1);
			m_frameCount = numSamps;
		}
	}
}

void ofxAlembic::IGeom::visit_geoms(ofPtr<IGeom> &obj, map<string, IGeom*> &object_name_map, map<string, IGeom*> &object_fullname_map)
{
	for (int i = 0; i < obj->m_children.size(); i++)
		visit_geoms(obj->m_children[i], object_name_map, object_fullname_map);
	
	if (obj->isTypeOf(UNKHOWN)) return;
	
//	assert(object_name_map.find(obj->getName()) == object_name_map.end());
	object_name_map[obj->getName()] = obj.get();
	
//	assert(object_fullname_map.find(obj->getFullName()) == object_fullname_map.end());
	object_fullname_map[obj->getFullName()] = obj.get();
}
