
#define FLT_MAX 3.402823466e+38F;

// Sample state.
SamplerState g_SampAni : register(s0);

// Render target.
RWTexture2D<float4> g_Target : register(u0);

cbuffer CONST_BUFFER : register(b0)
{
    // Frame width in pixels.
    const int g_width;
    // Frame height in pixels.
    const int g_height;
    float2 pad;
}

// Vertex point.
struct Vertex 
{
    // Position.
    float3 pos;
    // Normal.
    float3 norm;
    // Uv.
    float2 uv;
};
// Vertex buffer.
StructuredBuffer<Vertex> g_VertexBuffer : register(t0);

// Point light.
struct PointLight
{
    // Position.
    float3 pos;
    // Color.
    float3 col;
    // Max distance.
    float maxDistance;
    float pad;
};
// Point light buffer.
StructuredBuffer<PointLight> g_PointLightBuffer : register(t1);

// Sphere.
struct Sphere
{
    // Position.
    float3 pos;
    // Color.
    float3 col;
    // Radius.
    float radius;
    float pad;
};
// Sphere buffer.
StructuredBuffer<Sphere> g_SphereBuffer : register(t2);

// Entity entries.
struct EntityEntry
{
    // Index to first vertex in vertex buffer.
    int offset;
    // Number of vertices in entity.
    int numVertices;
    float2 pad;
};
// Vertex buffer.
StructuredBuffer<EntityEntry> g_EntityEntries : register(t3);

// Meta data.
struct MetaData 
{
    // Number of vertices in vertex buffer.
    int numVertices;
    // Number of lights in point light buffer.
    int numPointLights;
    // Number of spheres in sphere buffer;
    int numSpheres;
    // Number of bounces.
    int numBounces;
    // Number of entities.
    int numEntities;
    // Amount of super sampling anti analyzing.
    int ssaa;
    float2 pad;
};
// Meta buffer.
StructuredBuffer<MetaData> g_MetaBuffer : register(t4);

// Diffuse texture array.
Texture2DArray<float4> g_DiffuseTextureArray : register(t5);

// Hit data.
struct HitData 
{
    // Scalar to closet ray intersection.
    float t;
    // Whether hit is sphere or not.
    bool sphere;
    // Index to sphere.
    int sphereID;
    // Offset to first vertex.
    int vertexOffset;
};

// Intersect ray vs scene.
// rayOrigin Origin of ray.
// rayDirection Direction of ray.
// Returns hitdata.
HitData RayVsScene(float3 rayOrigin, float3 rayDirection);

// Intersect ray vs triangle.
// rayOrigin Origin of ray.
// rayDirection Direction of ray.
// v0, v1, v2 Vertex positions.
// Returns scalar to closest hit point.
float RayVsTriangle(float3 rayOrigin, float3 rayDirection, float3 v0, float3 v1, float3 v2);

// Intersect ray vs sphere.
// rayOrigin Origin of ray.
// rayDirection Direction of ray.
// position Position of sphere.
// radius Radius of sphere.
// Returns scalar to closest hit point.
float RayVsSphere(float3 rayOrigin, float3 rayDirection, float3 position, float radius);

// Interpolates vertex values.
// hitPoint Point of intersection.
// v0, v1, v2 Vertex points to interpolate.
// Returns interpolated vertex.
Vertex Interpolate(float3 hitPoint, Vertex v0, Vertex v1, Vertex v2);

// Calculate color for vertex.
// hitPoint Point of intersection.
// vertex Interpolated vertex.
// entityID Index of which entity vertex corresponds.
// Returns color.
float3 CalculateColor(float3 rayDirecetion, float3 hitPoint, float3 position, float3 normal, float3 diffuse);

// Get id of which entity this vertex belongs to.
// vertexIndex Index of vertex.
// Return id.
int GetEntityID(int vertexIndex);

// One thread for each pixel in screen.
[numthreads(32, 32, 1)]
void CS_main(uint3 threadID : SV_DispatchThreadID)
{
    // Contants.
    const int ssaa = g_MetaBuffer[0].ssaa;
    const int width = g_width;
    const int height = g_height;
    const int numBounces = g_MetaBuffer[0].numBounces;
    const float fov = 3.14f / 4.f;
    const float focalDistance = 0.1f;
    const float3 focalPoint = float3(0.f, 0.f, -focalDistance);
    const float aspectRatio = float(width) / height;
    const float heightbyWidth = 1.f / aspectRatio;
    const float R = tan(fov / 2.f) * focalDistance;
    const float U = heightbyWidth * R;

    const float pxWidth = 2.f * R / width;
    const float pxHeight = 2.f * U / height;
    const float pxX = pxWidth / (ssaa + 1);
    const float pxY = pxHeight / (ssaa + 1);
    const float pxXOffset = pxWidth * threadID.x;
    const float pxYOffset = pxHeight * threadID.y;

    float3 finalColor = float3(0.f, 0.f, 0.f);
    for (int y = 1; y <= ssaa; ++y)
    {
        for (int x = 1; x <= ssaa; ++x)
        {
            // Ray.
            float3 rayOrigin = float3(-R + (pxXOffset + pxX * x), U - (pxYOffset + pxY * y), 0.f);
            float3 rayDirection = normalize(rayOrigin - focalPoint);

            for (int i = 0; i < numBounces; ++i)
            {
                // Intersect ray vs scene.
                HitData hitData = RayVsScene(rayOrigin, rayDirection);
                float3 hitPoint = rayOrigin + rayDirection * hitData.t;

                // Get point data.
                float3 position;
                float3 normal;
                float3 diffuse;
                if (hitData.sphere)
                {  // Sphere.
                    Sphere sphere = g_SphereBuffer[hitData.sphereID];
                    position = hitPoint;
                    normal = normalize(hitPoint - sphere.pos);
                    diffuse = sphere.col;
                }
                else
                {   // Triangle.
                    int entityID = GetEntityID(hitData.vertexOffset);
                    // Interpolate vertex.
                    Vertex vertex = Interpolate(hitPoint, g_VertexBuffer[hitData.vertexOffset], g_VertexBuffer[hitData.vertexOffset + 1], g_VertexBuffer[hitData.vertexOffset + 2]);
                    position = vertex.pos;
                    normal = vertex.norm;
                    diffuse = g_DiffuseTextureArray.SampleLevel(g_SampAni, float3(vertex.uv, entityID), 0).xyz;
                }

                // Get point color.
                float energyFactor = pow(0.85f, i + 1); // Energy loss for each bounce.
                finalColor += CalculateColor(rayDirection, hitPoint, position, normal, diffuse) * energyFactor;

                // Bounce ray.
                rayDirection = reflect(rayDirection, normal);
                rayOrigin = hitPoint + rayDirection * 0.01f;
            }
        }
    }

    // Set pixel color.
    g_Target[threadID.xy] = float4(finalColor/(ssaa * ssaa), 1.f);
}

HitData RayVsScene(float3 rayOrigin, float3 rayDirection) 
{
    HitData hitData;
    hitData.t = FLT_MAX;
    hitData.sphere = false;

    // Ray vs Triangle.
    const int numVertices = g_MetaBuffer[0].numVertices;
    for (int v = 0; v < numVertices; v += 3)
    {
        float t = RayVsTriangle(rayOrigin, rayDirection, g_VertexBuffer[v].pos, g_VertexBuffer[v + 1].pos, g_VertexBuffer[v + 2].pos);
        if (t > -0.001f && t < hitData.t) 
        {
            hitData.t = t;
            hitData.vertexOffset = v;
        }
    }

    // Ray vs Sphere.
    const int numSpheres = g_MetaBuffer[0].numSpheres;
    for (int s = 0; s < numSpheres; ++s)
    {
        Sphere sphere = g_SphereBuffer[s];
        float t = RayVsSphere(rayOrigin, rayDirection, sphere.pos, sphere.radius);
        if (t > -0.001f && t < hitData.t)
        {
            hitData.t = t;
            hitData.sphere = true;
            hitData.sphereID = s;
        }
    }

    return hitData;
}

float RayVsTriangle(float3 rayOrigin, float3 rayDirection, float3 v0, float3 v1, float3 v2) 
{
    float3 e1 = v1 - v0;
    float3 e2 = v2 - v0;
    float3 s = rayOrigin - v0;
    float3 d = -rayDirection;
    float det = determinant(float3x3(d, e1, e2));
    if (abs(det) > 0.001f)
    {
        det = 1 / det;
        float u = det * determinant(float3x3(d, s, e2));
        float v = det * determinant(float3x3(d, e1, s));

        if ((u >= -0.001f && u <= 1.001f) && (v >= -0.001f && v <= (1.001f - u)))
        {
            return det * determinant(float3x3(s, e1, e2));
        }
    }
    return -1.f;

    //float3 E1 = v1 - v0;
    //float3 E2 = v2 - v0;
    //float3 n = -normalize(cross(E1, E2));
    //float t = dot(n, -rayDirection);
    //if (abs(t) > 0.00001f) {
    //    float d = -dot(n, v0);
    //    t = (d + dot(n, rayOrigin)) / t;
    //    float3 hitPoint = rayOrigin + rayDirection * t;
    //    float a = dot(cross(v1 - v0, hitPoint - v0), -n);
    //    float b = dot(cross(v2 - v1, hitPoint - v1), -n);
    //    float c = dot(cross(v0 - v2, hitPoint - v2), -n);
    //    if (a > -0.00001f && b > -0.00001f && c > -0.00001f)
    //    {
    //        return t;
    //    }
    //}
    //return -1.f;
}

float RayVsSphere(float3 rayOrigin, float3 rayDirection, float3 position, float radius)
{
    float3 oc = rayOrigin - position;
    float b = dot(rayDirection, oc);

    float p = b * b - (dot(oc, oc) - radius * radius);
    if (p >= 0.f) {
        p = sqrt(p);
        b = -b;

        float t1 = b + p;
        float t2 = b - p;

        if (t1 < t2 && t1 > 0.f)
        {
            return t1;
        }
        //t2 < t1
        else if (t2 > 0.f)
        {
            return t2;
        }
    }
    return -1.f;
}

Vertex Interpolate(float3 hitPoint, Vertex v0, Vertex v1, Vertex v2)
{
    float3 f = hitPoint;
    float3 p0 = v0.pos;
    float3 p1 = v1.pos;
    float3 p2 = v2.pos;

    // Calculate vectors from point f to vertices p0, p1 and p2.
    float3 f0 = p0 - f;
    float3 f1 = p1 - f;
    float3 f2 = p2 - f;
    // Calculate the areas and factors (order of parameters doesn't matter).
    float a = length(cross(p0 - p1, p0 - p2)); // main triangle area a.
    float a0 = length(cross(f1, f2)) / a; // p0's triangle area / a.
    float a1 = length(cross(f2, f0)) / a; // p1's triangle area / a.
    float a2 = length(cross(f0, f1)) / a; // p2's triangle area / a.

    Vertex vertex;
    vertex.pos = v0.pos * a0 + v1.pos * a1 + v2.pos * a2;
    vertex.norm = v0.norm * a0 + v1.norm * a1 + v2.norm * a2;
    vertex.uv = v0.uv * a0 + v1.uv * a1 + v2.uv * a2;

    return vertex;
}

float3 CalculateColor(float3 rayDirection, float3 hitPoint, float3 position, float3 normal, float3 diffuse)
{
    float3 color = float3(0.f, 0.f, 0.f);
    //sample.reflectionFactor = 0.f;
    int numOfPointLights = g_MetaBuffer[0].numPointLights;
    for (int i = 0; i < numOfPointLights; ++i)
    {
        // Point light data.
        PointLight pointLight = g_PointLightBuffer[i];
        float3 lightVec = pointLight.pos - position;
        float distance = length(lightVec);
        lightVec = normalize(pointLight.pos - position);
        float3 camVec = normalize(-position);
        float3 lightRefVec = normalize(reflect(-lightVec, normal));
        float specularFactor = max(dot(camVec, lightRefVec), 0.0f);
        specularFactor = pow(specularFactor, 5.f);

        // Shadow ray.
        HitData hitData = RayVsScene(hitPoint + lightVec * 0.01f, lightVec);

        if (hitData.t >= distance)
        {
            // Calculate diffuse.
            const float intensity = 15.f;
            float distanceFactor = clamp(intensity / (distance * distance), 0.f, 1.f);
            float diffuseFactor = dot(lightVec, normal);

            // Calculate specular.
            float3 specular = pointLight.col * specularFactor;
            const float specularIntensity = (1.f - diffuseFactor);

            // Combine color.
            color += (diffuse * diffuseFactor + specularIntensity * specular) * distanceFactor;
        }
    }

    return color;
}

int GetEntityID(int vertexIndex)
{
    int numEntities = g_MetaBuffer[0].numEntities;
    for (int i = 0; i < numEntities; ++i)
    {
        int offset = g_EntityEntries[i].offset;
        int numVertices = g_EntityEntries[i].numVertices;
        if (vertexIndex >= offset && vertexIndex < offset + numVertices)
            return i;
    }
    return -1;
}
