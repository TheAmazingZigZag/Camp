/********************************************************************************
The content of the files in this repository include portions of the AUDIOKINETIC
Wwise Technology released in source code form as part of the SDK package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use these files in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Copyright (c) 2021 Audiokinetic Inc.
********************************************************************************/


/*=============================================================================
AkAcousticPortalVisualizer.cpp:
=============================================================================*/

#include "AkAcousticPortalVisualizer.h"
#include "AkAcousticPortal.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "Components/BrushComponent.h"
#include "Classes/EditorStyleSettings.h"
#include "EditorModes.h"
#include "Materials/Material.h"

void UAkPortalComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	AAkAcousticPortal* pPortal = (AAkAcousticPortal*)Component->GetOwner();
	UBrushComponent* pBrComp = pPortal->GetBrushComponent();
	if (pBrComp && pBrComp->Brush)
	{
		FLinearColor SelectedColor = GetDefault<UEditorStyleSettings>()->SelectionColor;
		SelectedColor.A = 0.35f;

		{
			FVector Positions[4] =
			{
				FVector(-100, -100, +100),
				FVector(-100, +100, +100),
				FVector(+100, +100, +100),
				FVector(+100, -100, +100)
			};
			FVector2D UVs[4] =
			{
				FVector2D(0,0),
				FVector2D(0,1),
				FVector2D(1,1),
				FVector2D(1,0),
			};

			// Then rotate this face 4 times
			FRotator FaceRotations[6];
			int sides = 0;
			FaceRotations[sides++] = FRotator(0, 0, 0);
			FaceRotations[sides++] = FRotator(90.f, 0, 0);
			FaceRotations[sides++] = FRotator(-90.f, 0, 0);
			FaceRotations[sides++] = FRotator(180.f, 0, 0);

            FDynamicMeshBuilder MeshBuilder(ERHIFeatureLevel::Type::ES3_1);

			for (int32 f = 0; f < sides; f++)
			{
				FMatrix FaceTransform = FRotationMatrix(FaceRotations[f]);

				int32 VertexIndices[4];
				for (int32 VertexIndex = 0; VertexIndex < 4; VertexIndex++)
				{
					VertexIndices[VertexIndex] = MeshBuilder.AddVertex(
						FaceTransform.TransformPosition(Positions[VertexIndex]),
						UVs[VertexIndex],
						FaceTransform.TransformVector(FVector(1, 0, 0)),
						FaceTransform.TransformVector(FVector(0, 1, 0)),
						FaceTransform.TransformVector(FVector(0, 0, 1)),
						FColor::White
					);
				}

				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[1], VertexIndices[2]);
				MeshBuilder.AddTriangle(VertexIndices[0], VertexIndices[2], VertexIndices[3]);
				MeshBuilder.AddTriangle(VertexIndices[2], VertexIndices[1], VertexIndices[0]);
				MeshBuilder.AddTriangle(VertexIndices[3], VertexIndices[2], VertexIndices[0]);
			}

			// Allocate the material proxy and register it so it can be deleted properly once the rendering is done with it.
#if UE_4_22_OR_LATER
			auto* renderProxy = GEngine->GeomMaterial->GetRenderProxy();
#else
			auto* renderProxy = GEngine->GeomMaterial->GetRenderProxy(false);
#endif
			FDynamicColoredMaterialRenderProxy* SelectedColorInstance = new FDynamicColoredMaterialRenderProxy(renderProxy, SelectedColor);
			PDI->RegisterDynamicResource(SelectedColorInstance);

			MeshBuilder.Draw(PDI, pPortal->GetTransform().ToMatrixWithScale(), SelectedColorInstance, SDPG_World, false, false);
		}
		{
			FVector Positions[4] =
			{
				FVector(-100, 0, +100),
				FVector(-100, 0, -100),
				FVector(+100, 0, -100),
				FVector(+100, 0, +100)
			};

			SelectedColor.A = 0.85f;

			for (int pos = 0; pos < 4; pos++)
				Positions[pos] = pPortal->GetTransform().TransformPosition(Positions[pos]);

			float Thickness = 2.0f;

			PDI->DrawLine(Positions[0], Positions[1], SelectedColor, SDPG_Foreground, Thickness);
			PDI->DrawLine(Positions[1], Positions[2], SelectedColor, SDPG_Foreground, Thickness);
			PDI->DrawLine(Positions[2], Positions[3], SelectedColor, SDPG_Foreground, Thickness);
			PDI->DrawLine(Positions[3], Positions[0], SelectedColor, SDPG_Foreground, Thickness);
		}
		{
			// draw a diagonal on left and right faces if the portal is closed
			if (pPortal->InitialState == AkAcousticPortalState::Closed)
			{
				FVector Positions[4] =
				{
					FVector(100, -100, 100),
					FVector(-100, -100, -100),
					FVector(-100, 100, 100),
					FVector(100, 100, -100)
				};

				SelectedColor.A = 0.85f;

				for (int pos = 0; pos < 4; pos++)
					Positions[pos] = pPortal->GetTransform().TransformPosition(Positions[pos]);

				float Thickness = 2.0f;

				PDI->DrawLine(Positions[0], Positions[1], SelectedColor, SDPG_Foreground, Thickness);
				PDI->DrawLine(Positions[2], Positions[3], SelectedColor, SDPG_Foreground, Thickness);
			}
		}
	}
}