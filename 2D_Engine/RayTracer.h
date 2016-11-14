#pragma once

#include "EntityEntry.h"
#include "PointLight.h"
#include "Vertex.h"
#include <vector>

class Renderer;
class Scene;
class Shader;
class Texture;

struct ID3D11ShaderResourceView;
struct ID3D11Buffer;

// Ray tracer application.
class RayTracer 
{
    public:
        // Constructor.
        // width Application width in pixels.
        // height Application height in pixels.
        // scene Pointer to which scene to render.
        RayTracer(unsigned int width, unsigned int height, Scene* scene);

        // Destructor.
        ~RayTracer();

        // Whether application is running or not.
        bool Running();

        // Update scene.
        // scene Scene to update.
        void Update(Scene& scene);

        // Render scene.
        // scene Scene to render.
        void Render(Scene& scene);

        // Max number of vertices.
        const int mMaxNumVertices = 5000;

        // Max number of point lights.
        const int mMaxNumPointLights = 10;

        // Number of bounces.
        int mNumBounces = 1;

        // Max number of entities.
        const int mMaxNumOfEntities = 10;

        // Renderer to render window.
        Renderer* mRenderer = nullptr;

    private:
        // Compute shader helper class. 
        Shader* mShader = nullptr;

        // Vector of vertex points.
        std::vector<Vertex> mVertices;

        // Structured buffer contaning vertex data.
        ID3D11ShaderResourceView* mVertexBuffer;

        // Vector of point lights.
        std::vector<PointLight> mPointLights;

        // Structured buffer contaning point light data.
        ID3D11ShaderResourceView* mPointLightBuffer;

        // Vector of entity entries.
        std::vector<EntityEntry> mEntityEntries;

        // Structured buffer contaning entity entry data.
        ID3D11ShaderResourceView* mEntityEntryBuffer;

        // Structured buffer contaning metadata.
        ID3D11ShaderResourceView* mMetaBuffer;

        //// Diffuse texture array buffer.
        //ID3D11ShaderResourceView* mDiffuseTexArray64x64Buffer;

        //// Normal textures.
        //std::vector<ID3D11ShaderResourceView*> mNormalSRVs;

        //// Normal texture buffer.
        //ID3D11ShaderResourceView* mNormalTexBuffer;

        //// Metal texture.
        //std::vector<ID3D11ShaderResourceView*> mMetalSRVs;

        //// Metal texture buffer.
        //ID3D11ShaderResourceView* mMetalTexBuffer;

        // Vector of shader resource views.
        std::vector<ID3D11ShaderResourceView*> mSRVs;

        // Constant buffer contaning constant data.
        ID3D11Buffer* mConstBuffer;

        // Meta data.
        struct MetaData {
            // Number of vertices.
            int numOfVertices;
            // Number of point lights.
            int numOfPointLights;
            // Number of bounces.
            int numBounces;
            // Number of entites.
            int numEntities;

            // Constructor.
            MetaData(int numOfVertices, int numOfPointLights, int numBounces, int numEntities);
        };

        struct ConstData {
            // Screen width in pixels.
            int width;
            // Screen height in pixels.
            int height;
            float pad[2];

            // Constructor.
            ConstData(int width, int height);
        };
};
