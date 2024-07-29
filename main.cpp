#include <Novice.h>
#include<cmath>
#include<assert.h>
#include "Matrix.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Sphere.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include<imgui.h>

static const int KRowHeight = 20;
static const int Kcolumnwidth = 60;

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;


struct Line {
	Vector3 origin;//!< 始点
	Vector3 diff;//!< 終点への差分ベクトル
};

struct Ray {
	Vector3 origin;//!< 始点
	Vector3 diff;//!< 終点への差分ベクトル
};

struct Plane {
	Vector3 normal;
	float distance;
};

struct Segment {
	Vector3 origin;//!< 始点
	Vector3 diff;//!< 終点
};
struct Triangle {
	Vector3 vertices[3];//!<頂点
};


const char kWindowTitle[] = "LE2B_03_ウエノ_ユウキ_タイトル";

void DrawGrid(const Matrix4x4& viewProiectionMatrix, const Matrix4x4& ViewportMatrix) {
	const float KGridHalfwidth = 2.0f;
	const uint32_t KSubdivision = 10;
	const float KGridEvery = (KGridHalfwidth * 2.0f) / float(KSubdivision);

	for (uint32_t xIndex = 0; xIndex <= KSubdivision; xIndex++) {
		float posX = -KGridHalfwidth + xIndex * KGridEvery;

		Vector3 startPointX(posX, 0.0f, -KGridHalfwidth);
		Vector3 endPointX(posX, 0.0f, KGridHalfwidth);

		startPointX = Transform(startPointX, Multiply(viewProiectionMatrix, ViewportMatrix));
		endPointX = Transform(endPointX, Multiply(viewProiectionMatrix, ViewportMatrix));
		if (xIndex == 5) {
			Novice::DrawLine((int)startPointX.x, (int)startPointX.y, (int)endPointX.x, (int)endPointX.y, BLACK);
		}
		else {
			Novice::DrawLine((int)startPointX.x, (int)startPointX.y, (int)endPointX.x, (int)endPointX.y, 0x6F6F6FFF);
		}
	}
	for (uint32_t zIndex = 0; zIndex <= KSubdivision; zIndex++) {
		float posZ = -KGridHalfwidth + KGridEvery * zIndex;

		Vector3 startPointZ = { -KGridHalfwidth, 0.0f, posZ };
		Vector3 endPointZ = { KGridHalfwidth, 0.0f, posZ };
		startPointZ = Transform(startPointZ, Multiply(viewProiectionMatrix, ViewportMatrix));
		endPointZ = Transform(endPointZ, Multiply(viewProiectionMatrix, ViewportMatrix));
		if (zIndex == 5) {
			Novice::DrawLine((int)startPointZ.x, (int)startPointZ.y, (int)endPointZ.x, (int)endPointZ.y, BLACK);
		}
		else {
			Novice::DrawLine((int)startPointZ.x, (int)startPointZ.y, (int)endPointZ.x, (int)endPointZ.y, 0x6F6F6FFF);
		}
	}

}
bool IsCollision(const Triangle& triangle, const Segment& segment) {

	Vector3 edge1 = Subtract(triangle.vertices[1], triangle.vertices[0]);
	Vector3 edge2 = Subtract(triangle.vertices[2], triangle.vertices[0]);

	Vector3 normal = Cross(edge1, edge2);
	normal = Normalize(normal);

	Vector3 dir = segment.diff;
	dir = Normalize(dir);

	Vector3 diff = Subtract(triangle.vertices[0], segment.origin);

	float dotND = Dot(normal, dir);
	if (fabs(dotND) < 1e-6f) {
		return false;
	}

	float t = Dot(normal, diff);
	if (t < 0.0f || t>Length(segment.diff)) {
		return false;
	}

	Vector3 intersection = Add(segment.origin, vMultiply(t, dir));

	Vector3 c0 = Cross(Subtract(triangle.vertices[1], triangle.vertices[0]), Subtract(intersection, triangle.vertices[0]));
	Vector3 c1 = Cross(Subtract(triangle.vertices[2], triangle.vertices[1]), Subtract(intersection, triangle.vertices[1]));
	Vector3 c2 = Cross(Subtract(triangle.vertices[0], triangle.vertices[2]), Subtract(intersection, triangle.vertices[2]));

	if (Dot(c0, normal) >= 0.0f && Dot(c1, normal) >= 0.0f && Dot(c2, normal) >= 0.0f) {

		return true;
	}
	return false;
}


Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}
///
///三角形の描画
///
void DrawTriangle(const Triangle& triangle, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	// 三角形の各頂点を変換する
	Vector3 transformedVertices[3];
	for (int32_t index = 0; index < 3; ++index) {
		transformedVertices[index] = Transform(Transform(triangle.vertices[index], viewProjectionMatrix), viewportMatrix);
	}

	// 描画
	Novice::DrawLine(static_cast<int>(transformedVertices[0].x), static_cast<int>(transformedVertices[0].y), static_cast<int>(transformedVertices[1].x), static_cast<int>(transformedVertices[1].y), color);
	Novice::DrawLine(static_cast<int>(transformedVertices[1].x), static_cast<int>(transformedVertices[1].y), static_cast<int>(transformedVertices[2].x), static_cast<int>(transformedVertices[2].y), color);
	Novice::DrawLine(static_cast<int>(transformedVertices[2].x), static_cast<int>(transformedVertices[2].y), static_cast<int>(transformedVertices[0].x), static_cast<int>(transformedVertices[0].y), color);
}



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Vector3 camaraTranslate = { 0.0f,1.9f,-6.49f };
	Vector3 cameraRotate = { 0.26f,0.0f,0.0f };

	Vector3 point{ -1.5f,0.6f,0.6f };


	Segment segment{ {-1.0f,-1.0f,0.0f},3.0f,1.0f,1.0f };

	Triangle triangle[] = { {0.0f,2.0f,0.0f},{2.0f,-2.0f,0.0f},{-2.0f,-2.0f,0.0f} };
	


	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);


		///
		/// ↓更新処理ここから
		///


		Matrix4x4 worldMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, { 0.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f });
		Matrix4x4 viewWorldMatrix = Inverse(worldMatrix);

		Matrix4x4 cameraMatrxi = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, camaraTranslate);
		Matrix4x4 viewCameraMatrix = Inverse(cameraMatrxi);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewWorldMatrix, Multiply(viewCameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		//Vector3 start = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		//Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		//Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);










		
		ImGui::Begin("Window");
		ImGui::DragFloat3("camaraTranslate", &camaraTranslate.x, 0.01f);
		ImGui::DragFloat3("camaraRotate", &cameraRotate.x, 0.01f);
		//ImGui::DragFloat3("sphere", &sphere.center.x, 0.01f);
		//ImGui::DragFloat("radius", &sphere.radius, 0.01f);
		ImGui::DragFloat3("segment", &segment.origin.x, 0.01f);
		//ImGui::DragFloat3("Plane", &plane.normal.x, 0.01f);
		//plane.normal = Normalize(plane.normal);
		ImGui::End();


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///






		DrawGrid(viewProjectionMatrix, viewportMatrix);// グリッドの描画
		for (int i = 0; i < 3; ++i) {
			//if (IsCollision(triangle[i], segment)) {
			//	DrawTriangle(triangle[i], viewProjectionMatrix, viewportMatrix, RED);
			//}
			//else {
			//	DrawTriangle(triangle[i], viewProjectionMatrix, viewportMatrix, WHITE);
			//}
			DrawTriangle(triangle[i], viewProjectionMatrix, viewportMatrix,WHITE);
		}


		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}