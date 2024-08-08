#include <Novice.h>
#include <cmath>
#include <assert.h>
#include "Matrix.h"
#include "Matrix4x4.h"
#include "Vector3.h"
#include "Spring.h"
#include "Ball.h"
#include "Sphere.h"
#include"pendulum.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include <imgui.h>

static const int KRowHeight = 20;
static const int Kcolumnwidth = 60;

static const int kWindowWidth = 1280;
static const int kWindowHeight = 720;

struct Plane {
	Vector3 normal;
	float distance;
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

static void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 12;							//分割数
	const float kLatStep = (float)M_PI / kSubdivision;			//緯度のステップ
	const float kLonStep = 2.0f * (float)M_PI / kSubdivision;	//経度のステップ

	// 緯度のループ
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -0.5f * (float)M_PI + latIndex * kLatStep;	//現在の緯度

		//次の緯度
		float nextLat = lat + kLatStep;

		//経度のループ
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
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

Vector3 Perpendicular(const Vector3& vector) {
	if (vector.x != 0.0f || vector.y != 0.0f) {
		return{ -vector.y,vector.x,0.0f };
	}
	return { 0.0f,-vector.z,vector.y };
}

void DrawPlane(const Plane& plane, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	Vector3 center = Multiply(plane.normal,plane.distance);// 1
	Vector3 perpendiculars[4];
	perpendiculars[0] = Normalize(Perpendicular(plane.normal));// 2
	perpendiculars[1] = { -perpendiculars[0].x,-perpendiculars[0].y,-perpendiculars[0].z };//3
	perpendiculars[2] = Cross(plane.normal, perpendiculars[0]);// 4
	perpendiculars[3] = { -perpendiculars[2].x,-perpendiculars[2].y,-perpendiculars[2].z };// 5
	// 6
	Vector3 points[4];
	for (int32_t index = 0; index < 4; ++index) {
		Vector3 extend = Multiply(perpendiculars[index], 2.0f);
		Vector3 point = Add(center, extend);
		points[index] = Transform(Transform(point, viewProjectionMatrix), viewportMatrix);
	}


	Novice::DrawLine(int(points[0].x), int(points[0].y), int(points[2].x), int(points[2].y), color);
	Novice::DrawLine(int(points[2].x), int(points[2].y), int(points[1].x), int(points[1].y), color);
	Novice::DrawLine(int(points[1].x), int(points[1].y), int(points[3].x), int(points[3].y), color);
	Novice::DrawLine(int(points[3].x), int(points[3].y), int(points[0].x), int(points[0].y), color);
}
Vector3 Reflect(const Vector3& input, const Vector3& normal) {

	float dotProduct = Dot(input, normal);

	// 反射ベクトルを計算
	Vector3 reflection = {
		input.x - normal.x * (2 * dotProduct),
		input.y - normal.y * (2 * dotProduct),
		input.z - normal.z * (2 * dotProduct)
	};

	return reflection;
}

// 球と平面の当たり判定
bool  IsCollision(const Sphere& sphere, const Plane& plane) {
	Vector3 normalizedNormal = Normalize(plane.normal);
	// 球の中心と平面の距離を計算
	float distance = Dot(normalizedNormal, sphere.center) - plane.distance;
	// 距離の絶対値が球の半径以下であれば衝突
	return std::abs(distance) <= sphere.radius;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	Plane plane{};
	plane.normal = Normalize({ -0.2f,0.9f,-0.3f });
	plane.distance = 0.0f;


	Ball ball{};
	ball.position = { 1.2f,1.2f,0.3f };
	ball.mass = 2.0f;
	ball.radius = 0.1f;
	ball.color = BLUE;
	ball.aceleration = { 0.0f,-9.8f,0.0f };

	Sphere sphere{};
	sphere.radius = 0.1f;

	float deltaTime = 1.0f / 60.0f;

	bool start = false;

	float e = 0.02f;


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

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewCameraMatrix = Inverse(cameraMatrix);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewWorldMatrix, Multiply(viewCameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);



		if (start) {
			ball.velocity.x += ball.aceleration.x * deltaTime;
			ball.velocity.y += ball.aceleration.y * deltaTime;
			ball.velocity.z += ball.aceleration.z * deltaTime;

			ball.position.x += ball.velocity.x * deltaTime;
			ball.position.y += ball.velocity.y * deltaTime;
			ball.position.z += ball.velocity.z * deltaTime;

			if (IsCollision(Sphere{ ball.position,ball.radius }, plane)) {
				Vector3 normalizedNormal = Normalize(plane.normal);
				float penetrationDepth = (ball.radius - Dot(normalizedNormal, ball.position) + plane.distance);

				// 球を平面から押す
				ball.position.x += normalizedNormal.x * penetrationDepth;
				ball.position.y += normalizedNormal.y * penetrationDepth;
				ball.position.z += normalizedNormal.z * penetrationDepth;

				Vector3 reflected = Reflect(ball.velocity, plane.normal);
				Vector3 projectToNormal = Project(reflected, plane.normal);
				Vector3 movingDirection = {
				reflected.x - projectToNormal.x,
				reflected.y - projectToNormal.y,
				reflected.z - projectToNormal.z,
				};

				ball.velocity.x = projectToNormal.x * e + movingDirection.x;
				ball.velocity.y = projectToNormal.y * e + movingDirection.y;
				ball.velocity.z = projectToNormal.z * e + movingDirection.z;
			}


		}



	



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
				float distance = Length(Subtract(cameraTranslate, cameraTarget.center));
				Matrix4x4 rotationMatrix = MakeRotateMatrix(cameraRotate);
				Vector3 offset = { 0.0f, 0.0f, -distance };
				cameraTranslate = Add(cameraTarget.center, Transform(offset, rotationMatrix));
			}

			// マウスの位置を更新
			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}


		ImGui::Begin("Window");
		ImGui::Checkbox("Start", &start);
		ImGui::End();

		/// 
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///
		
		DrawGrid(viewProjectionMatrix, viewportMatrix);

		DrawPlane(plane, viewProjectionMatrix, viewportMatrix, WHITE);

		DrawSphere(Sphere{ball.position,ball.radius}, viewProjectionMatrix, viewportMatrix, WHITE);

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
