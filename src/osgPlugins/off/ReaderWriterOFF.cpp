#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

#include <iostream>
#include <fstream>
#include <string>

#include <stdio.h>
#include <string.h>

class WriterOFFNodeVisitor: public osg::NodeVisitor {

    public:
        WriterOFFNodeVisitor(std::ostream& fout) :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _fout(fout)
        {
           // _fout << "# file written by OpenSceneGraph" << std::endl << std::endl;
        }

        virtual void apply(osg::Geode &node);

    protected:

        WriterOFFNodeVisitor& operator = (const WriterOFFNodeVisitor&) { return *this; }
        std::ostream& _fout;

};

void WriterOFFNodeVisitor::apply( osg::Geode &node )
{
    osg::Matrix matrix = osg::computeLocalToWorld(getNodePath());

    unsigned int count = node.getNumDrawables();
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *geometry = node.getDrawable( i )->asGeometry();
        if ( geometry )
        {
            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
            osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
            osg::Vec3Array* colours = dynamic_cast<osg::Vec3Array*>(geometry->getColorArray());

            if ( vertices ) {
                for (unsigned int ii=0;ii<vertices->size();ii++) {

                    // update nodes with world coords
                    osg::Vec3d v = vertices->at(ii) * matrix;
                    _fout << v[0] << ' ' << v[1] << ' ' << v[2];

                    if ( colours )
                    {
                        v=colours->at(ii);
                        _fout << ' ' << (int)v[0]*255.0 << ' ' << (int)v[1]*255.0 << ' ' << (int)v[2]*255.0;
                    }
                    else
                    {
                        _fout << " 255 255 255";
                    }

                    if ( normals )
                    {
                        v = normals->at(ii);
                        _fout << ' ' << v[0] << ' ' << v[1] << ' ' << v[2];
                    }
                    else
                    {
                        _fout << " 0.0 0.0 1.0";
                    }


                    _fout << std::endl;
                }
            }

        }
    }
}

enum ReaderState {
    READ_HEADER,
    READ_COUNT,
    READ_DIM,
    READ_VERTICES,
    READ_FACES
};

using namespace std;

class ReaderWriterOFF : public osgDB::ReaderWriter
{
    public:

        ReaderWriterOFF()
        {
            supportsExtension("off","Geomview Object File Format");
        }

        virtual const char* className() const { return "OFF Geomview Object File Format Reader"; }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO << "Reading file "<<fileName<<std::endl;

            std::string line;

            // unsigned int targetNumVertices = 10000;

            osg::Geode* geode = new osg::Geode;

            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            osg::Vec3Array* normals = new osg::Vec3Array;
            osg::Vec4ubArray* colours = new osg::Vec4ubArray;

            osg::Vec3 pos;
            osg::Vec3 normal(0.0,0.0,1.0);
            int r=255,g=255,b=255,a=255;
            char sep;


            bool has_header = false;
            bool has_normals = false;
            bool has_colours = false;
            bool has_st = false;
            bool has_w = false;
            bool has_dim = false;
            bool dim = 3;
            
            int  vertex_count, face_count, edge_count;
            int  vertex_idx, face_idx, edge_idx;
            vertex_idx = face_idx = edge_idx = 0;
            
            ReaderState reader_state = READ_HEADER;
            osgDB::ifstream fin(fileName.c_str());
            while (std::getline(fin, line)) {
            {
                if (line[0]=='#')
                {
                    // comment line
                    OSG_INFO <<"Comment: "<<line<<std::endl;
                    continue;
                }
                if (line.size() == 0) continue;
                
                switch (reader_state) {
                    case READ_HEADER :
                    {
                        const char * cp = line.c_str();
                        if (strncmp(cp, "ST", 2) == 0) {
                            has_st = true;
                            cp += 2;
                        }
                        if (cp[0] == 'N') {
                            has_normals = true;
                            cp ++;
                        }
                        if (cp[0] == 'C') {
                            has_colours = true;
                            cp ++;
                        }
                        if (cp[0] == '4') {
                            has_w = true;
                            cp ++;
                        }
                        if (cp[0] == 'n') {
                            has_dim = true;
                            cp ++;
                        }
                        if (strncmp(cp, "OFF", 3) != 0) {
                            OSG_INFO << "Error in file : " << line << std::endl;
                            return NULL;
                        }
                        if (has_dim) {
                            reader_state = READ_DIM;
                        } else {
                            reader_state = READ_COUNT;
                        }
                    }
                    break;
                    
                    case READ_DIM:
                    {
                        
                    }
                    break;
                    
                    case READ_COUNT:
                    {
                        vertex_count = face_count = edge_count = 0;
                        int matched = sscanf(line.c_str(),"%d%d%d",
                                             &vertex_count, &face_count, &edge_count);
                        if (matched != 3) {
                            OSG_INFO << "Error in file : " << line << std::endl;
                            return NULL;
                        }
                        reader_state = READ_VERTICES;
                    }
                    break;
                    
                    case READ_VERTICES:
                    {
                        std::vector<float> all_floats;
                        std::istringstream is( line );
                        std::copy(std::istream_iterator<float>(is),
                                  std::istream_iterator<float>(),
                                  std::back_inserter(all_floats));
                        
                        if (all_floats.size() < 3) {
                            OSG_INFO << "Error in file : " << line << std::endl;
                            return NULL;
                        }
                        osg::Vec3 pos(all_floats[0], all_floats[1], all_floats[2]);
                        vertices->push_back(pos);
                        
                        if (has_colours) {
                            if (all_floats.size() < 6) {
                                OSG_INFO << "Error in file : " << line << std::endl;
                                return NULL;
                            }
                            osg::Vec3 colour(all_floats[3], all_floats[4], all_floats[5]);
//                            colours->push_back(colour);
                        }
                        
                        vertex_idx ++;
                        if (vertex_idx == vertex_count) {
                            reader_state = READ_FACES;
                        }
                    }
                    break;
                        
                    case READ_FACES:
                    {
                        osg::DrawElementsUInt* pDrawElements = new osg::DrawElementsUInt(GL_POLYGON);
                        std::vector<unsigned int> all_integers;
                        
                        std::istringstream is( line );
                        std::copy(std::istream_iterator<unsigned int>(is),
                                  std::istream_iterator<unsigned int>(),
                                  std::back_inserter(all_integers));
                        
                        for (int i=1; i<all_integers.size(); i++) {
                            unsigned int v = all_integers[i];
                            pDrawElements->push_back(v);
                        }
                        geometry->addPrimitiveSet(pDrawElements);
                    }
                    break;
                        
                }
            }
                
#if 0                
                    if (!has_header) {
                    if (strncmp(line, "OFF", 3) == 0) {
                        continue;
                    }
                    
                    if (v == -1) {
                        matched = sscanf(line, "%d %d %d", &numvertices, &numfaces, &numedges);
                    } else if (f == -1) {
                        matched = sscanf(line, "")
                    } else if (e == -1) {
                        
                    }
                    
                    
                    int matched = sscanf(line,"%f%c%f%c%f%c%d%c%d%c%d%c%f%c%f%c%f",
                                   &pos.x(),&sep,&pos.y(),&sep,&pos.z(),&sep,
                                   &r,&sep,&g,&sep,&b,&sep,
                                   &normal.x(),&sep,&normal.y(),&sep,&normal.z());

                    if (matched)
                    {
                        if (vertices->size()>=targetNumVertices)
                        {
                            // finishing setting up the current geometry and add it to the geode.
                            geometry->setUseDisplayList(true);
                            geometry->setUseVertexBufferObjects(true);
                            geometry->setVertexArray(vertices);
                            geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
                            geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
                            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

                            geode->addDrawable(geometry);

                            // allocate a new geometry
                            geometry = new osg::Geometry;

                            vertices = new osg::Vec3Array;
                            normals = new osg::Vec3Array;
                            colours = new osg::Vec4ubArray;

                            vertices->reserve(targetNumVertices);
                            normals->reserve(targetNumVertices);
                            colours->reserve(targetNumVertices);

                        }

                        vertices->push_back(pos);
                        normals->push_back(normal);
                        colours->push_back(osg::Vec4ub(r,g,b,a));
                    }
                }
#endif
            }


            geometry->setUseDisplayList(true);
            geometry->setUseVertexBufferObjects(true);
            geometry->setVertexArray(vertices);
            if (has_colours) geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
            if (has_normals) geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);

            geode->addDrawable(geometry);

            return geode;

        }

        virtual WriteResult writeNode(const osg::Node& node,const std::string& fileName,const Options* options =NULL) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if( !acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream f(fileName.c_str());

            WriterOFFNodeVisitor nv(f);

            // we must cast away constness
            (const_cast<osg::Node*>(&node))->accept(nv);

            return WriteResult::FILE_SAVED;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(off, ReaderWriterOFF)
