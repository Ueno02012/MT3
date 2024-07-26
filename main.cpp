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
bool IsCollision(const Segment& segment, const Plane& plane) {
	float dot = Dot(plane.normal, segment.diff);

	if (dot == 0.0f) {
		return false;
	}

	float t = (plane.distance - Dot(segment.origin, plane.normal)) / dot;
	

	return (0.0f <= t && t <= 1.0f);
}


Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}
void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = vMultiply(plane.distance, plane.normal);// 1
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));// 2
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y,-perpendiculars[0].z };//3
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);// 4
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };// 5
	// 6
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = vMultiply(2.0f, perpendiculars[index]);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}


	Novice::DrawLine(int(points[0].x), int(points[0].y),int(points[2].x),int(points[2].y),color);
	Novice::DrawLine(int(points[2].x), int(points[2].y),int(points[1].x),int(points[1].y),color);
	Novice::DrawLine(int(points[1].x), int(points[1].y),int(points[3].x),int(points[3].y),color);
	Novice::DrawLine(int(points[3].x), int(points[3].y),int(points[0].x),int(points[0].y),color);
}


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Vector3 camaraTranslate = { 0.0f,1.9f,-6.49f };
	Vector3 cameraRotate = { 0.26f,0.0f,0.0f };

	Vector3 point{ -1.5f,0.6f,0.6f };

	//Sphere sphere{};
	////Sphere sphere2{};
	Plane plane{};

	//sphere.radius = 0.5f;
	////sphere2.center.x = 1.0f;
	////sphere2.radius = 0.5;
	plane.normal = { 0.0f,1.0f,0.0f };
	plane.distance = 1.0f;

	//Segment segment{ {-2.0f,-1.0f,0.0f},3.0f,2.0f,2.0f };





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

		Vector3 start = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);
		Novice::DrawLine(int(start.x), int(start.y), int(end.x), int(end.y), WHITE);


		//IsCollision(sphere, plane);








		
		ImGui::Begin("Window");
		ImGui::DragFloat3("camaraTranslate", &camaraTranslate.x, 0.01f);
		ImGui::DragFloat3("camaraRotate", &cameraRotate.x, 0.01f);
		//ImGui::DragFloat3("sphere", &sphere.center.x, 0.01f);
		//ImGui::DragFloat("radius", &sphere.radius, 0.01f);
		ImGui::DragFloat3("Plane.Normal", &plane.normal.x, 0.01f);
		plane.normal = Normalize(plane.normal);
		ImGui::End();


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///






		DrawGrid(viewProjectionMatrix, viewportMatrix);// グリッドの描画
		//if (IsCollision(sphere,plane)) {
		//	DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, RED);// 球体の描画
		//}
		//else {
		//	DrawSphere(sphere, viewProjectionMatrix, viewportMatrix, WHITE);// 球体の描画
		//}
		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);
		//DrawSphere(sphere2, viewProjectionMatrix, viewportMatrix, WHITE);// 球体の描画




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