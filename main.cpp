#include <Novice.h>
#include<cmath>
#include<assert.h>
#include "Matrix.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Sphere.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include<imgui.h>

static const int KRowHeight = 20;
static const int Kcolumnwidth = 60;

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

struct Segment {
	Vector3 origin;//!< 始点
	Vector3 diff;//!< 終点
};


struct AABB
{
	Vector3 min;//!<始点
	Vector3 max;//!<終点
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
bool IsCollision(const AABB& aabb, const Sphere& sphere) {
	//　最近接点を求める
	Vector3 closestPoint{ 
		std::clamp(sphere.center.x,aabb.min.x,aabb.max.x),
		std::clamp(sphere.center.y,aabb.min.y,aabb.max.y),
		std::clamp(sphere.center.z,aabb.min.z,aabb.max.z),
	};
	// 最近接点と弾の中心との距離を求める
	float distance = Length(Subtract(closestPoint, sphere.center));

	// 距離が半径よりも小さければ衝突
	if (distance <= sphere.radius) {
		// 衝突
		return true;
	}
	return false;
}
static void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color)
{
	const uint32_t kSubdivision = 12;							//分割数
	const float kLatStep = (float)M_PI / kSubdivision;			//緯度のステップ
	const float kLonStep = 2.0f * (float)M_PI / kSubdivision;	//経度のステップ

	// 緯度のループ
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex)
	{
		float lat = -0.5f * (float)M_PI + latIndex * kLatStep;	//現在の緯度

		//次の緯度
		float nextLat = lat + kLatStep;

		//経度のループ
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex)
		{
			//現在の経度
			float lon = lonIndex * kLonStep;

			//次の経度
			float nextLon = lon + kLonStep;

			// 球面座標の計算
			Vector3 pointA
			{
				sphere.center.x + sphere.radius * cos(lat) * cos(lon),
				sphere.center.y + sphere.radius * sin(lat),
				sphere.center.z + sphere.radius * cos(lat) * sin(lon)
			};

			Vector3 pointB
			{
				sphere.center.x + sphere.radius * cos(nextLat) * cos(lon),
				sphere.center.y + sphere.radius * sin(nextLat),
				sphere.center.z + sphere.radius * cos(nextLat) * sin(lon)
			};

			Vector3 pointC
			{
				sphere.center.x + sphere.radius * cos(lat) * cos(nextLon),
				sphere.center.y + sphere.radius * sin(lat),
				sphere.center.z + sphere.radius * cos(lat) * sin(nextLon)
			};

			// スクリーン座標に変換
			pointA = Transform(pointA, Multiply(viewProjectionMatrix, viewportMatrix));
			pointB = Transform(pointB, Multiply(viewProjectionMatrix, viewportMatrix));
			pointC = Transform(pointC, Multiply(viewProjectionMatrix, viewportMatrix));

			// 線分の描画
			Novice::DrawLine((int)pointA.x, (int)pointA.y, (int)pointB.x, (int)pointB.y, color);
			Novice::DrawLine((int)pointA.x, (int)pointA.y, (int)pointC.x, (int)pointC.y, color);
		}
	}
}

void DrawAABB(const AABB& aabb, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 vertexces[8] = {
		{aabb.min.x,aabb.min.y,aabb.min.z},// 0 手前の左下の点
		{aabb.max.x,aabb.min.y,aabb.min.z},// 1 手前の右下の点
		{aabb.max.x,aabb.min.y,aabb.max.z},// 2 奥の右下の点
		{aabb.min.x,aabb.min.y,aabb.max.z},// 3 奥の左下の点
		{aabb.min.x,aabb.max.y,aabb.min.z},// 4
		{aabb.max.x,aabb.max.y,aabb.min.z},// 5
		{aabb.max.x,aabb.max.y,aabb.max.z},// 6
		{aabb.min.x,aabb.max.y,aabb.max.z},

	};
		
	for (int32_t index = 0; index < 8; ++index) {
		vertexces[index] = Transform(Transform(vertexces[index], viewProjectionMatrix), viewportMatrix);
	}
	Novice::DrawLine(int(vertexces[0].x), int(vertexces[0].y), int(vertexces[1].x), int(vertexces[1].y), color);
	Novice::DrawLine(int(vertexces[1].x), int(vertexces[1].y), int(vertexces[2].x), int(vertexces[2].y), color);
	Novice::DrawLine(int(vertexces[2].x), int(vertexces[2].y), int(vertexces[3].x), int(vertexces[3].y), color);
	Novice::DrawLine(int(vertexces[3].x), int(vertexces[3].y), int(vertexces[0].x), int(vertexces[0].y), color);

	Novice::DrawLine(int(vertexces[4].x), int(vertexces[4].y), int(vertexces[5].x), int(vertexces[5].y), color);
	Novice::DrawLine(int(vertexces[5].x), int(vertexces[5].y), int(vertexces[6].x), int(vertexces[6].y), color);
	Novice::DrawLine(int(vertexces[6].x), int(vertexces[6].y), int(vertexces[7].x), int(vertexces[7].y), color);
	Novice::DrawLine(int(vertexces[7].x), int(vertexces[7].y), int(vertexces[4].x), int(vertexces[4].y), color);

	Novice::DrawLine(int(vertexces[0].x), int(vertexces[0].y), int(vertexces[4].x), int(vertexces[4].y), color);
	Novice::DrawLine(int(vertexces[1].x), int(vertexces[1].y), int(vertexces[5].x), int(vertexces[5].y), color);
	Novice::DrawLine(int(vertexces[2].x), int(vertexces[2].y), int(vertexces[6].x), int(vertexces[6].y), color);
	Novice::DrawLine(int(vertexces[3].x), int(vertexces[3].y), int(vertexces[7].x), int(vertexces[7].y), color);


}

///
///カメラの位置
///
Matrix4x4 LookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
	Vector3 zaxis = Normalize(Subtract(target , eye));    // 前方向ベクトル
	Vector3 xaxis = Normalize(Cross(up, zaxis)); // 右方向ベクトル
	Vector3 yaxis = Cross(zaxis, xaxis);        // 上方向ベクトル

	Matrix4x4 viewMatrix = {
		xaxis.x, yaxis.x, zaxis.x, 0,
		xaxis.y, yaxis.y, zaxis.y, 0,
		xaxis.z, yaxis.z, zaxis.z, 0,
		-Dot(xaxis, eye), -Dot(yaxis, eye), -Dot(zaxis, eye), 1
	};

	return viewMatrix;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	//Vector3 camaraTranslate = { 0.0f,1.9f,-6.49f };
	//Vector3 cameraRotate = { 0.26f,0.0f,0.0f };

	Vector3 point{ -1.5f,0.6f,0.6f };

	//Sphere sphere{};

	//sphere.radius = 0.5f;
	//sphere.center.x = 1.0f;

	Segment segment{ {-1.0f,-1.0f,0.0f},-1.0f,0.0f,3.0f };


	// カメラ行列
	Vector3 cameraTranslate{ 0.0f, 1.9f, -10.49f };
	Vector3 cameraRotate{ 0.26f, 0.0f, 0.0f };
	Sphere cameraTarget;
	cameraTarget.center = { 0.0f, 0.0f, 0.0f }; // カメラのターゲットポイント
	cameraTarget.radius = 0.01f;
	int lastMouseX = 0;
	int lastMouseY = 0;
	int mouseX = 0;
	int mouseY = 0;
	bool IsDebugCameraActive = false;

	
	AABB aabb1{
		.min{-0.5f,-0.5f,-0.5f},
		.max{0.0f,0.0f,0.0f},
	};




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

		Matrix4x4 cameraMatrxi = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewCameraMatrix = Inverse(cameraMatrxi);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewWorldMatrix, Multiply(viewCameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);



		

		/// ===デバックカメラ起動=== ///
		if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
			if (IsDebugCameraActive) {
				IsDebugCameraActive = false;
			}
			else {
				IsDebugCameraActive = true;
			}
		}

		/// ===デバックカメラ起動=== ///
		// デバッグカメラが有効になっている場合、マウスの動きによってカメラを回転させる
		if (IsDebugCameraActive) {
			Novice::GetMousePosition(&mouseX, &mouseY);

			if (Novice::IsPressMouse(0)) {
				// マウスの移動量を計算
				int deltaX = mouseX - lastMouseX;
				int deltaY = mouseY - lastMouseY;

				// カメラの回転を更新
				float rotationSpeed = 0.005f;
				cameraRotate.y += deltaX * rotationSpeed;
				cameraRotate.x += deltaY * rotationSpeed;

				// カメラの位置をターゲットポイントの周りに回転
				float distance = Length(Subtract(cameraTranslate , cameraTarget.center));
				Matrix4x4 rotationMatrix = MakeRotateMatrix(cameraRotate);
				Vector3 offset = { 0.0f, 0.0f, -distance };
				cameraTranslate = Add(cameraTarget.center , Transform(offset, rotationMatrix));
			}

			// マウスの位置を更新
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}

		

		
		ImGui::Begin("Window");
		ImGui::DragFloat3("camaraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("camaraRotate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("aabb1.min", &aabb1.min.x, 0.01f);
		ImGui::DragFloat3("aabb1.max", &aabb1.max.x, 0.01f);


		ImGui::End();


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///


		DrawGrid(viewProjectionMatrix, viewportMatrix);// グリッドの描画

		DrawAABB(aabb1, viewProjectionMatrix, viewportMatrix, WHITE);

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