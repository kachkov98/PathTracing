#pragma once
#include "3dmath.h"
#include "image.h"
#include "log.h"

enum Material
{
	MAT_DIFF,
	MAT_SPEC,
	MAT_REFR
};

struct Sphere
{
	double r;
	Material mat;
	Vec3 position, emission, color;
	Sphere ()
	{
	}
	Sphere (Vec3 Position, double R, Material Mat, Vec3 Color, Vec3 Emission) :
		position (Position),
		r (R),
		mat (Mat),
		color (Color),
		emission (Emission)
	{
	}
};

class Scene
{
private:
	Sphere *spheres;
	size_t *lights;
	size_t num_spheres;
	size_t num_lights;
	Ray camera;
	Vec3 cam_x, cam_y;
	double fov;
	double near_z;
	Vec3 Raycast (Ray &ray, unsigned int depth, double is_e = 1.0);
	bool GetInfo (Ray &ray, double &min_z, size_t &object_id);
public:
	Scene (const char *filename);
	void Render (Image &image, unsigned int num_samples);
	~Scene ();
};