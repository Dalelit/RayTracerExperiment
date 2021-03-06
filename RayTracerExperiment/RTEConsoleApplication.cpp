// Defines the entry point for the console application.
//

#include "stdafx.h"
#include "time.h"
#include "RayTracerExperiment.h"

void CreateBMPFile(Bitmap bitmap, LPCWSTR filename)
{
	int pixelCount = bitmap.width * bitmap.height;
	int bmpSize = pixelCount * sizeof(Pixel);

	//// set BMP file and info header
	BITMAPFILEHEADER fileHdr;
	BITMAPINFOHEADER infoHdr;

	fileHdr.bfType = 0x4D42; //BM
	fileHdr.bfSize = bmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	fileHdr.bfReserved1 = 0;
	fileHdr.bfReserved2 = 0;
	fileHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

	infoHdr.biSize = sizeof(BITMAPINFOHEADER);
	infoHdr.biWidth = bitmap.width;
	infoHdr.biHeight = bitmap.height;
	infoHdr.biPlanes = 1;
	infoHdr.biBitCount = 32;
	infoHdr.biCompression = BI_RGB;
	infoHdr.biSizeImage = 0;
	infoHdr.biXPelsPerMeter = 0;
	infoHdr.biYPelsPerMeter = 0;
	infoHdr.biClrUsed = 0;
	infoHdr.biClrImportant = 0;

	//// Write file

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
	{
		printf("Error creating file\n");
		return;
	}

	DWORD dwBytesWritten = 0;

	if (!WriteFile(hFile, (LPSTR)&fileHdr, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL)) printf("Error writting file header.\n");
	//printf("Wrote %d bytes to file.\n", dwBytesWritten);

	if (!WriteFile(hFile, (LPSTR)&infoHdr, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL)) printf("Error writting file info.\n");
	//printf("Wrote %d bytes to file.\n", dwBytesWritten);

	if (!WriteFile(hFile, (LPSTR)bitmap.pixels, BitMapArraySize(bitmap), &dwBytesWritten, NULL)) printf("Error writting bmp array.\n");
	//printf("Wrote %d bytes to file.\n", dwBytesWritten);

	CloseHandle(hFile);
}

void SetBitmapTestPattern(Bitmap bitmap)
{
	int halfW = bitmap.width / 2;
	int halfH = bitmap.height / 2;

	Pixel* pix = bitmap.pixels;
	for (int y = 0; y < bitmap.height; y++)
	{
		for (int x = 0; x < bitmap.width; x++)
		{
			if (y < halfH) pix->r = 255;
			else pix->r = 0;

			if (x < halfW) pix->g = 255;
			else pix->g = 0;

			pix->b = 0;
			pix->a = 0;
			pix++;
		}
	}
}

void SetBitmapColour(Bitmap bitmap, UINT8 r, UINT8 g, UINT8 b)
{
	Pixel* pix = bitmap.pixels;
	for (int y = 0; y < bitmap.height; y++)
	{
		for (int x = 0; x < bitmap.width; x++)
		{
			pix->r = r;
			pix->g = g;
			pix->b = b;
			pix->a = 0;
			pix++;
		}
	}
}

struct FragmentQueue
{
	volatile int nextFragment;
	int totalFragments;
	Fragment* fragments;
	HANDLE mutex;
	Stats stats;
};

int GetNextFragmentSafely(FragmentQueue* queue)
{
	WaitForSingleObject(queue->mutex, INFINITE);
	int result = queue->nextFragment++; // get the current value and increment
	ReleaseMutex(queue->mutex);
	return result;
}

void UpdateFragmentStatsSafely(FragmentQueue* queue, Stats stats)
{
	WaitForSingleObject(queue->mutex, INFINITE);
	queue->stats.hitCount = stats.hitCount;
	queue->stats.rayCount = stats.rayCount;
	ReleaseMutex(queue->mutex);
}

// thread function
DWORD WINAPI ProcessFragments(LPVOID args)
{
	FragmentQueue* queue = (FragmentQueue*)args;
	Stats stats = {};

	int indx = GetNextFragmentSafely(queue);

	while (indx < queue->totalFragments)
	{
		TraceFragment(&queue->fragments[indx]);
		stats.rayCount += queue->fragments[indx].stats.rayCount;
		stats.hitCount += queue->fragments[indx].stats.hitCount;
		indx = GetNextFragmentSafely(queue);
	}

	UpdateFragmentStatsSafely(queue, stats);

	return 0;
}

void DoSingleTrace(Bitmap bitmap, World* world, Camera* camera)
{
	Ray ray;
	ray.origin = camera->position;
	XMVECTOR point = camera->lookingAt + XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	ray.direction = XMVector3Normalize(point - camera->position);

	HitInfo hitInfo;

	bool hit = TraceWorld(world, &ray, &hitInfo);

	if (hit)
	{
		printf("direction ");
		PrintXMVector3F32(ray.direction);
		printf("\n");

		printf("-direction ");
		PrintXMVector3F32(-ray.direction);
		printf("\n");

		printf("Normal ");
		PrintXMVector3F32(hitInfo.normal);
		printf("\n");

		printf("Hitpoisition ");
		PrintXMVector3F32(hitInfo.position);
		printf("\n");

		XMVECTOR v = XMVector3Reflect(ray.direction, hitInfo.normal);
		printf("new direction ");
		PrintXMVector3F32(v);
		printf("\n");
	}
	else
	{
		printf("Missed.\n");
	}
}

void DoSubSetSingleThread(Bitmap bitmap, World* world, Camera* camera)
{
	Fragment frag;

	frag.xStart = bitmap.width / 2;
	frag.yStart = bitmap.height / 2;
	frag.xUpTo = frag.xStart + 2;
	frag.yUpTo = frag.yStart + 2;
	frag.world = world;
	frag.camera = camera;
	frag.bitmap = bitmap;
	frag.seed = 123456;

	TraceFragment(&frag);
}

void DoFullPictureThreaded(Bitmap bitmap, World* world, Camera* camera)
{
	int fragCols = 8;
	int fragRows = 2;
	int threadCount = 8;

	//// establish the work queues
	FragmentQueue workQueue;
	workQueue.mutex = CreateMutex(NULL, false, NULL); //TODO: add checking that it worked?!
	workQueue.stats = {};
	workQueue.totalFragments = fragCols * fragRows;
	workQueue.nextFragment = 0;
	workQueue.fragments = (Fragment*)malloc(workQueue.totalFragments * sizeof(Fragment));

	int fragWidth = bitmap.width / fragCols;
	int fragHeight = bitmap.height / fragRows;

	Fragment* f = workQueue.fragments;
	for (int row = 0; row < fragRows; row++)
	{
		int yStart = row * fragHeight;
		int yUpTo = (row + 1) * fragHeight;
		if (row + 1 == fragRows) yUpTo = bitmap.height; // end row... always make it the bitmap height

		for (int col = 0; col < fragCols; col++)
		{
			f->bitmap = bitmap;
			f->world = world;
			f->camera = camera;
			f->xStart = col * fragWidth;
			f->xUpTo = (col + 1) * fragWidth;
			if (col + 1 == fragCols) f->xUpTo = bitmap.width; // end column... always make it the bitmap width
			f->yStart = yStart;
			f->yUpTo = yUpTo;

			f->seed = rand();

			f++;
		}
	}

	//// write out config
	printf("Threads: %i\n", threadCount);
	printf("Fragments: %i\n", workQueue.totalFragments);


	//// kick off the threads to do the work

	DWORD*  threadIds = (DWORD*)malloc(threadCount * sizeof(DWORD));
	HANDLE* threads = (HANDLE*)malloc(threadCount * sizeof(HANDLE));

	clock_t startTime = clock();

	for (int i = 0; i < threadCount; i++)
	{
		threads[i] = CreateThread(
			NULL,
			0,
			ProcessFragments,
			&workQueue,
			0,
			&threadIds[i]
		);
	}

	WaitForMultipleObjects(threadCount, threads, true, INFINITE);
	clock_t finishTime = clock();


	//// write out stats
	long timeTaken = (finishTime - startTime) * 1000 / CLOCKS_PER_SEC;
	printf("Done. %ims\n", timeTaken);
	printf("Ray count: %I64i\n", workQueue.stats.rayCount);
	if (workQueue.stats.rayCount > 0) printf("Ray hit count: %I64i (%f%%)\n", workQueue.stats.hitCount, (float)workQueue.stats.hitCount / (float)workQueue.stats.rayCount * 100.0f);
	printf("Rays per ms: %f\n", (float)workQueue.stats.rayCount / (float)timeTaken);
	printf("ms per pixel: %f\n", (float)timeTaken / (float)(bitmap.height * bitmap.width));
	printf("Pixels per ms: %f\n", (float)(bitmap.height * bitmap.width) / (float)timeTaken);

	//// clean up
	CloseHandle(workQueue.mutex);
	free(workQueue.fragments);
	free(threadIds);
	free(threads);

}

int main()
{
	//RunQuickTest();

	//// set the config
	// To Do: make this external config file or input args?
	Bitmap bitmap;
	//bitmap.width = 800; bitmap.height = 600;
	bitmap.width = 1920; bitmap.height = 1080;
	float aspectRatio = (float)bitmap.height / (float)bitmap.width;
	LPCWSTR filename = TEXT("output.bmp");

	//// create BMP
	bitmap.pixels = (Pixel*)malloc(BitMapArraySize(bitmap));

	if (!bitmap.pixels)
	{
		printf("Error malloc bitmap pixels.\n");
		return -1;
	}

	//SetBitmapTestPattern(bitmap);
	//SetBitmapColour(bitmap, 255, 0, 0);


	//// set the scene
	World world;
	Camera camera;

	world.config.maxBounces = 8;
	world.config.samples = 8;

	InitialiseTestWorld1(&world);
	InitialiseTestCamera1(&camera, aspectRatio);

	printf("Starting...\n");
	printf("Filename: %S\n", filename);
	printf("Samples: %i\n", world.config.samples);
	printf("Max bounces: %i\n", world.config.maxBounces);

	//DoSingleTrace(bitmap, &world, &camera);
	DoFullPictureThreaded(bitmap, &world, &camera);
	//DoSubSetSingleThread(bitmap, &world, &camera);

	//// write out file
	CreateBMPFile(bitmap, filename);

	//// clean up
	free(bitmap.pixels);


	//// done!
	return 0;
}
