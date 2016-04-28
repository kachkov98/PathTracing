#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>
#include "scene.h"

Vec3 world_color (0.0, 0.0, 0.5);
const double far_z = 1000000000;
const double ior = 1.6;
const double fresnel = 1.0;

Scene::Scene (const char *filename)
{
	FILE *f;
	fopen_s (&f, filename, "r");
	if (f)
		Log ("File %s opened successfully", filename);
	else
	{
		Log ("Can not open file %s", filename);
		exit (1);
	}
	fscanf_s (f, "%lf %lf %lf %lf %lf %lf %lf %lf ",
			  &camera.start.x, &camera.start.y, &camera.start.z,
			  &camera.dir.x, &camera.dir.y, &camera.dir.z, &near_z, &fov);
	camera.dir.norm ();

	num_spheres = 0;
	num_lights = 0;
	fscanf_s (f, "%zu ", &num_spheres);
	spheres = new Sphere[num_spheres];
	for (size_t i = 0; i < num_spheres; i++)
	{
		Vec3 pos, color, emission;
		double r;
		char mat[5];
		fscanf_s (f, "%lf %lf %lf %lf %s %lf %lf %lf %lf %lf %lf",
				  &pos.x, &pos.y, &pos.z, &r, mat, 5,
				  &color.x, &color.y, &color.z,
				  &emission.x, &emission.y, &emission.z);

		if (emission.x > DBL_EPSILON || emission.y > DBL_EPSILON || emission.z > DBL_EPSILON)
			num_lights++;

		spheres[i].position = pos;
		spheres[i].r = r;
		if (!strcmp (mat, "SPEC"))
			spheres[i].mat = MAT_SPEC;
		else if (!strcmp (mat, "REFR"))
			spheres[i].mat = MAT_REFR;
		else
			spheres[i].mat = MAT_DIFF;
		spheres[i].color = color;
		spheres[i].emission = emission;
	}

	lights = new size_t [num_lights];
	size_t cur_light = 0;
	for (size_t i = 0; i < num_spheres; i++)
		if (spheres[i].emission.x > DBL_EPSILON || spheres[i].emission.y > DBL_EPSILON || spheres[i].emission.z > DBL_EPSILON)
			lights[cur_light++] = i;
	Log ("Scene loaded successfully");
}

bool Scene::GetInfo (Ray &ray, double &min_z, size_t &object_id)
{
	bool is_intersect = false;
	min_z = far_z;
	for (size_t i = 0; i < num_spheres; i++)
	{
		Vec3 oc = spheres[i].position - ray.start;
		double b2 = ray.dir ^ oc;
		double c = (oc ^ oc) - (spheres[i].r * spheres[i].r);
		double det = b2 * b2 - c;
		if (det < 0.0)
			continue;
		det = sqrt (det);
		const double eps = 0.0001;
		double z1 = b2 - det;
		if (z1 > eps && z1 < min_z)
		{
			min_z = z1;
			is_intersect = true;
			object_id = i;
		}
		else
		{
			z1 = b2 + det;
			if (z1 > eps && z1 < min_z)
			{
				min_z = z1;
				is_intersect = true;
				object_id = i;
			}
		}
	}
	return is_intersect;
}

Vec3 Scene::Raycast (Ray &ray, unsigned int depth, double is_e)
{
	Vec3 position, normal, color, emission;
	size_t object_id;
	bool into = true;
	double min_z;
	if (!GetInfo (ray, min_z, object_id))
		return world_color;
	position = ray.start + ray.dir * min_z;
	normal = (position - spheres[object_id].position) * (1.0 / spheres[object_id].r);
	if ((normal ^ ray.dir) > DBL_EPSILON)
	{
		normal *= -1.0;
		into = false;
	}
	color = spheres[object_id].color;
	emission = spheres[object_id].emission;

	//find maximum color component
	double max_rgb = (color.x > color.y && color.x > color.z) ? color.x : (color.y > color.z) ? color.y : color.z;

	if (++depth > 5 || max_rgb < DBL_EPSILON)
	{
		if (drand () < max_rgb)
			color *= (1.0 / max_rgb);
		else
			return emission * is_e;
	}

	if (spheres[object_id].mat == MAT_DIFF)
	{
		double angle1 = 2.0 * M_PI * drand (), angle2 = 0.5 * M_PI * drand ();
		Vec3 w = normal;
		Vec3 u = ((fabs (w.x) > 0.1 ? Vec3 (0.0, 1.0, 0.0) : Vec3 (1.0, 0.0, 0.0)) % w);
		Vec3 v = w % u;
		Vec3 direction = (u * fast_cos(angle1 - M_PI) + v * fast_sin (angle1 - M_PI) + w * fast_sin (angle2)).norm();

		Vec3 light_e;
		for (size_t i = 0; i < num_lights; i++)
		{
			Sphere &s = spheres[lights[i]];

			Vec3 lw = (s.position - position).norm();
			Vec3 lu = ((fabs (lw.x) > 0.1 ? Vec3 (0.0, 1.0, 0.0) : Vec3 (1.0, 0.0, 0.0)) % lw);
			Vec3 lv = lw % lu;
			double cos_a_max = sqrt (1 - s.r * s.r / ((position - s.position) ^ (position - s.position)));
			double eps1 = drand (), eps2 = drand ();
			double cos_a = 1 - eps1 + eps1 * cos_a_max;
			double sin_a = sqrt (1 - cos_a*cos_a);
			double phi = 2 * M_PI * eps2 - M_PI;
			Vec3 l = lu * fast_cos (phi) * sin_a + lv * fast_sin (phi) * sin_a + lw * cos_a;
			double z;
			size_t id;
			if (GetInfo (Ray (position, l), z, id) && id == lights[i])
			{
				double omega = 2 * M_PI * (1 - cos_a_max);
				light_e += color * s.emission * (l ^ normal) * omega * M_1_PI;
			}
		}
		return emission * is_e + light_e + color * Raycast (Ray (position, direction), depth, 0.0);
	}
	else if (spheres[object_id].mat == MAT_SPEC)
	{
		return emission + color * Raycast (Ray (position, ray.dir - normal * (normal ^ ray.dir) * 2.0), depth);
	}
	//glass
	Ray refl_ray (position, ray.dir - normal * (normal ^ ray.dir) * 2.0);
	double ior_koef = (into) ? 1.0 / ior : ior;
	double ray_normal = ray.dir ^ normal;
	double cos2t = 1 - ior_koef * ior_koef * (1.0 - ray_normal * ray_normal);
	if (cos2t < 0.0)
		return emission + color * Raycast (refl_ray, depth);
	Vec3 tdir = (ray.dir * ior_koef - normal *(ray_normal * ior_koef + sqrt (cos2t))).norm ();
	double a = ior - fresnel;
	double b = ior + fresnel;
	double R0 = (a * a) / (b * b);
	double c = 1.0  + (into ? ray_normal : (tdir ^ normal));
	double Re = R0 + (1 - R0) * c * c * c * c * c;
	double P = 0.25 + 0.5 * Re;
	double RP = Re / P;
	double TP = (1.0 - Re) / (1.0 - P);

	if (depth > 2)
	{
		if (drand () < P)
			return emission + color * Raycast (refl_ray, depth) * RP;
		else
			return emission + color * Raycast (Ray (position, tdir), depth) * TP;
	}
	else
		return emission + color * (Raycast (refl_ray, depth) * Re + Raycast (Ray (position, tdir), depth) * (1 - Re));
}

void Scene::Render (Image &image, unsigned int num_samples)
{
	cam_x = Vec3 (image.width * fov / image.height);
	cam_y = (cam_x % camera.dir).norm () * fov;
	drand_init ((unsigned int)time (NULL));
	#pragma omp parallel
	{
		for (unsigned int y = 0; y < image.height; y++)
		{
			printf ("\r Rendering: %d %%", y * 100 / image.height);
			for (unsigned int x = 0; x < image.width; x++)
			{
				size_t offset = (y * image.width + x) * 3;
				Vec3 color;
				for (unsigned int sx = 0; sx < 2; sx++)
					for (unsigned int sy = 0; sy < 2; sy++)
					{
						Vec3 radiance;
						for (unsigned int samples = 0; samples < num_samples; samples++)
						{
							double r1 = 2.0 * drand (), r2 = 2.0 * drand ();
							double dx = (r1 < 1.0) ? sqrt (r1) - 1.0 : 1.0 - sqrt (2.0 - r1);
							double dy = (r2 < 1.0) ? sqrt (r2) - 1.0 : 1.0 - sqrt (2.0 - r2);
							Vec3 d = cam_x * (((sx + 0.5 + dx) / 2 + x) / image.width - 0.5) +
								cam_y * (((sy + 0.5 + dy) / 2 + y) / image.height - 0.5) +
								camera.dir;
							radiance += Raycast (Ray (camera.start + d * near_z, d.norm ()), 0);
						}
						radiance *= (1.0 / num_samples);
						color += (Vec3 (clamp (radiance.x), clamp (radiance.y), clamp (radiance.z)) * 0.25);
					}
				image.image[offset + 0] = gamma (color.z);
				image.image[offset + 1] = gamma (color.y);
				image.image[offset + 2] = gamma (color.x);
			}
		}
	}
}

Scene::~Scene ()
{
	delete[] spheres;
	delete[] lights;
}