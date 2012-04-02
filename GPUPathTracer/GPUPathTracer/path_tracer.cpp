#include "path_tracer.h"

#include "image.h"
#include "sphere.h"
#include "ray.h"
#include "rendercamera.h"

#include "windows_include.h"

// System:
#include <stdio.h>
#include <cmath>
#include <ctime>
#include <iostream>

// CUDA:
#include <cuda_runtime.h>
#include "cutil_math.h"

#include "cuda_safe_call.h"

#include "path_tracer_kernel.h"



PathTracer::PathTracer() {

	image = newImage(512, 512); // TODO: Don't hard-code this.

	// TODO: better way to set up camera/make it not hard-coded
	rendercam = new RenderCamera;
	setUpCamera(rendercam);

	setUpScene();

	createDeviceData();

}

void PathTracer::Reset(){
	image = newImage(512, 512); // TODO: Don't hard-code this.
}

PathTracer::~PathTracer() {

	deleteDeviceData();
	deleteImage(image);

}

void PathTracer::setUpCamera(RenderCamera* cam){
	// TODO: better way to set up camera/make it not hard-coded
	cam->position = make_float3(0.0, 0.0, 4.8);
	cam->view = normalize( make_float3(0.0, 0.0, -1.0) );
	cam->up = normalize( make_float3(0.0, 1.0, 0.0) );
	cam->fov = make_float2(40,40); // Derive one based on the other.
	cam->resolution = make_float2(image->width, image->height); // Setting to image size for now, to avoid duplicate definition that we have to manually keep in sync.
}

Image* PathTracer::render() {
	Image* singlePassImage = newImage(image->width, image->height);

	launch_kernel(numSpheres, spheres, singlePassImage->numPixels, singlePassImage->pixels, rays, image->passCounter, rendercam);

	// TODO: Make a function for this (or a method---maybe Image can just be a class).
	for (int i = 0; i < image->numPixels; i++) {
		image->pixels[i] += singlePassImage->pixels[i];
	}
	image->passCounter++;

	deleteImage(singlePassImage);

	return image;
}

void PathTracer::setUpScene() {

	numSpheres = 6; // TODO: Move this!

}

void PathTracer::createDeviceData() {

    // Initialize data:

	Sphere* tempSpheres = new Sphere[numSpheres];

	Ray* tempRays = new Ray[image->numPixels];




	// TEMPORARY hard-coded spheres:

	/*tempSpheres[0].position = make_float3(-0.9, 0, -0.3);
	tempSpheres[0].radius = 0.8;
	tempSpheres[0].diffuseColor = make_float3(0.87, 0.15, 0.15);
	tempSpheres[0].emittedColor = make_float3(0, 0, 0);

	tempSpheres[1].position = make_float3(0.8, 0, -0.8);
	tempSpheres[1].radius = 0.8;
	tempSpheres[1].diffuseColor = make_float3(0.15, 0.87, 0.15);
	tempSpheres[1].emittedColor = make_float3(0, 0, 0);

	tempSpheres[2].position = make_float3(1.3, 1.6, -2.3);
	tempSpheres[2].radius = 0.8;
	tempSpheres[2].diffuseColor = make_float3(0, 0, 0);
	tempSpheres[2].emittedColor = make_float3(5, 5, 5.4);*/

	tempSpheres[0].position = make_float3(-0.9, 0, 0.3);
	tempSpheres[0].radius = 0.8;
	tempSpheres[0].diffuseColor = make_float3(0.87, 0.15, 0.15);
	tempSpheres[0].emittedColor = make_float3(0, 0, 0);

	tempSpheres[1].position = make_float3(0.8, 0, -0.8);
	tempSpheres[1].radius = 0.8;
	tempSpheres[1].diffuseColor = make_float3(0.15, 0.87, 0.15);
	tempSpheres[1].emittedColor = make_float3(0, 0, 0);

	tempSpheres[2].position = make_float3(1.3, 1.6, -2.3);
	tempSpheres[2].radius = 0.8;
	tempSpheres[2].diffuseColor = make_float3(0, 0, 0);
	tempSpheres[2].emittedColor = make_float3(5, 5, 5.4);

	tempSpheres[3].position = make_float3(-0.2, 2, -0.3);
	tempSpheres[3].radius = 0.3;
	tempSpheres[3].diffuseColor = make_float3(0.87, 0.15, 0.15);
	tempSpheres[3].emittedColor = make_float3(0, 0, 0);

	tempSpheres[4].position = make_float3(-1.8, .6, -1.8);
	tempSpheres[4].radius = 1.2;
	tempSpheres[4].diffuseColor = make_float3(0.15, 0.87, 0.15);
	tempSpheres[4].emittedColor = make_float3(0, 0, 0);

	tempSpheres[5].position = make_float3(-1.3, 2.2, -2.3);
	tempSpheres[5].radius = 0.3;
	tempSpheres[5].diffuseColor = make_float3(0, 0, 0);
	tempSpheres[5].emittedColor = make_float3(5, 5, 5.4);

	//tempSpheres[2].emittedColor = make_float3(0, 0, 0);


    // Copy to GPU:
	CUDA_SAFE_CALL( cudaMalloc( (void**)&spheres, numSpheres * sizeof(Sphere) ) );
	CUDA_SAFE_CALL( cudaMalloc( (void**)&rays, image->numPixels * sizeof(Ray) ) );
    CUDA_SAFE_CALL( cudaMemcpy( spheres, tempSpheres, numSpheres * sizeof(Sphere), cudaMemcpyHostToDevice) );
    CUDA_SAFE_CALL( cudaMemcpy( rays, tempRays, image->numPixels * sizeof(Ray), cudaMemcpyHostToDevice) );

    delete [] tempSpheres;
	delete [] tempRays;
}

void PathTracer::deleteDeviceData() {
    CUDA_SAFE_CALL( cudaFree( spheres ) );
    CUDA_SAFE_CALL( cudaFree( rays ) );

}


