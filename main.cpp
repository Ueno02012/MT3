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

 Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t) {
	 return Vector3(
		 { v1.x + (v2.x - v1.x) * t },
		 { v1.y + (v2.y - v1.y) * t },
		 { v1.z + (v2.z - v1.z) * t }
	);
}
 // ベジエ曲線上の点を計算する関数
 Vector3 QuadraticBezier(const Vector3& P0, const Vector3& P1, const Vector3& P2, float t) {
	 Vector3 a = Lerp(P0, P1, t);
	 Vector3 b = Lerp(P1, P2, t);
	 return Lerp(a, b, t);
 }


 // クリップ空間の座標をビューポート空間の座標に変換する関数（簡易的な実装）
 Vector3 ViewportTransform(const Vector3& v, const Matrix4x4& viewportMatrix) {
	 return Transform(v, viewportMatrix);
 }

 void DrawBezier(const Vector3& controlPoint0, const Vector3& controlPoint1, const Vector3& controlPoint2,
	 const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {

	 // ベジエ曲線の分割数を設定
	 const uint32_t segments = 100;

	 // 始点をスクリーンに変換
	 Vector3 prevPoint = Transform(controlPoint0, viewProjectionMatrix);
	 prevPoint = ViewportTransform(prevPoint, viewportMatrix);

	 // 分割数に従ってベジエ曲線を描画
	 for (uint32_t i = 1; i <= segments; ++i) {
		 // 現在の分割位置を計算
		 float t = static_cast<float>(i) / segments;

		 // ベジエ曲線上の現在の点を計算
		 Vector3 point = QuadraticBezier(controlPoint0, controlPoint1, controlPoint2, t);

		 // 現在の点をクリップ空間の座標に変換
		 point = Transform(point, viewProjectionMatrix);

		 // クリップ空間の座標をビューポート空間の座標に変換
		 point = ViewportTransform(point, viewportMatrix);

		 Novice::DrawLine(int(prevPoint.x), int(prevPoint.y), int(point.x), int(point.y), color);
		 
		 prevPoint = point;
	 }
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


	Vector3 point{ -1.5f,0.6f,0.6f };

	Vector3 P0 = { 0, 0, 0 };
	Vector3 P1 = { 1, 2, 0 };
	Vector3 P2 = { 2, 0, 0 };
	Vector3 P3 = { 3, 2, 0 };

	//float t = 0.5f;

	Segment segment{ 
		.origin{-0.7f,0.3f,0.0f},
		.diff{2.0f,-0.5f,0.0f}
	};

	Vector3 controlPoints[3] = {
		{-0.8f,0.58f,1.0f},
		{1.76f,1.0f,-0.3f},
		{0.94f,-0.7f,2.3f},
	};

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

		Matrix4x4 cameraMatrxi = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewCameraMatrix = Inverse(cameraMatrxi);

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewWorldMatrix, Multiply(viewCameraMatrix, projectionMatrix));
		Matrix4x4 viewportMatrix = MakeViewportMatrix(0.0f, 0.0f, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);



		Vector3 start = Transform(Transform(segment.origin, viewProjectionMatrix), viewportMatrix);
		Vector3 end = Transform(Transform(Add(segment.origin, segment.diff), viewProjectionMatrix), viewportMatrix);


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
		ImGui::DragFloat3("controlPoints[0]", &controlPoints[0].x, 0.01f);
		ImGui::DragFloat3("controlPoints[1]", &controlPoints[1].x, 0.01f);
		ImGui::DragFloat3("controlPoints[2]", &controlPoints[2].x, 0.01f);
		ImGui::DragFloat3("segment", &segment.origin.x, 0.01f);


		ImGui::End();


		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///


		DrawGrid(viewProjectionMatrix, viewportMatrix);// グリッドの描画
		DrawBezier(controlPoints[0], controlPoints[1], controlPoints[2], viewProjectionMatrix, viewportMatrix, BLUE);

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