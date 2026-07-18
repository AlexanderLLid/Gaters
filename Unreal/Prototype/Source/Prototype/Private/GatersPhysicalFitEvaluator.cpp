#include "GatersPhysicalFitEvaluator.h"

#include "GatersTerrainSemanticField.h"
#include "GatersWorldCompiler.h"

namespace
{
void AddIssue(
	FGatersPhysicalFitEvaluation& Evaluation,
	const FGatersPhysicalFitCandidate& Candidate,
	const TCHAR* RuleId,
	const FString& SubjectId,
	FString Message,
	double Measured = 0.0,
	double Limit = 0.0,
	const FString& ObstacleId = FString())
{
	FGatersPhysicalFitIssue& Issue = Evaluation.Issues.AddDefaulted_GetRef();
	Issue.RuleId = RuleId;
	Issue.RecipeId = Candidate.RecipeId;
	Issue.AssetId = Candidate.Contract.AssetId;
	Issue.SubjectId = SubjectId;
	Issue.ObstacleId = ObstacleId;
	Issue.Message = MoveTemp(Message);
	Issue.Measured = Measured;
	Issue.Limit = Limit;
}

FBox ContractBox(
	const FVector& Center,
	const FVector& Extent,
	float CentimetersPerUnit,
	const FTransform& Transform)
{
	const FVector CenterCm = Center * CentimetersPerUnit;
	const FVector ExtentCm = Extent * CentimetersPerUnit;
	return FBox(CenterCm - ExtentCm, CenterCm + ExtentCm).TransformBy(Transform);
}

bool HasVolume(const FVector& Extent)
{
	return Extent.X > 0.f && Extent.Y > 0.f && Extent.Z > 0.f;
}
}

FGatersPhysicalFitCandidate FGatersPhysicalFitEvaluator::SampleTerrain(
	const FString& RecipeId,
	const FGatersAssetContract& Contract,
	const FTransform& Transform,
	const FGatersEnvironment& Environment,
	float PadRadius,
	const FVector2D& RouteTarget,
	float NormalSampleDistance)
{
	check(NormalSampleDistance > 0.f);
	FGatersPhysicalFitCandidate Candidate;
	Candidate.RecipeId = RecipeId;
	Candidate.Contract = Contract;
	Candidate.Transform = Transform;
	for (const FGatersAssetContact& Contact : Contract.Contacts)
	{
		if (Contact.Support != EGatersAssetContactSupport::Terrain)
		{
			continue;
		}
		FGatersPhysicalFitContactSample& Sample = Candidate.ContactSamples.AddDefaulted_GetRef();
		Sample.ContactName = Contact.Name;
		Sample.WorldLocation = Transform.TransformPosition(
			Contact.Location * Contract.CentimetersPerUnit);
		const FVector2D Point(Sample.WorldLocation);
		Sample.TerrainHeight = FGatersTerrainSemanticField::MaterializedHeightAt(
			Environment, Point, PadRadius, RouteTarget);
		Sample.TerrainNormal = FGatersTerrainSemanticField::MaterializedNormalAt(
			Environment, Point, NormalSampleDistance, PadRadius, RouteTarget);
	}
	return Candidate;
}

FGatersPhysicalFitEvaluation FGatersPhysicalFitEvaluator::Evaluate(
	const FGatersPhysicalFitCandidate& Candidate,
	const FGatersPhysicalFitSettings& Settings)
{
	FGatersPhysicalFitEvaluation Evaluation;
	TArray<FString> ContractErrors;
	if (!Candidate.Contract.Validate(ContractErrors))
	{
		AddIssue(Evaluation, Candidate, TEXT("fit.contract.invalid"),
			Candidate.Contract.AssetId, TEXT("asset contract is invalid"));
		return Evaluation;
	}

	for (const FGatersAssetContact& Contact : Candidate.Contract.Contacts)
	{
		if (Contact.Support == EGatersAssetContactSupport::Attachment)
		{
			++Evaluation.PendingAttachmentContacts;
			continue;
		}
		++Evaluation.EvaluatedTerrainContacts;
		const FGatersPhysicalFitContactSample* Sample = Candidate.ContactSamples.FindByPredicate(
			[&Contact](const FGatersPhysicalFitContactSample& Value)
			{
				return Value.ContactName == Contact.Name;
			});
		if (!Sample)
		{
			AddIssue(Evaluation, Candidate, TEXT("fit.contact.evidence_missing"), Contact.Name,
				TEXT("contact has no terrain evidence"));
			continue;
		}

		const double Gap = Sample->WorldLocation.Z - Sample->TerrainHeight;
		if (Gap > Settings.ContactToleranceCm)
		{
			AddIssue(Evaluation, Candidate, TEXT("fit.contact.floating"), Contact.Name,
				FString::Printf(TEXT("contact floats by %.3f cm"), Gap),
				Gap, Settings.ContactToleranceCm);
		}
		else if (Gap < -Settings.ContactToleranceCm)
		{
			AddIssue(Evaluation, Candidate, TEXT("fit.contact.buried"), Contact.Name,
				FString::Printf(TEXT("contact is buried by %.3f cm"), -Gap),
				-Gap, Settings.ContactToleranceCm);
		}

		const FVector ContactNormal = Candidate.Transform.TransformVectorNoScale(Contact.Normal).GetSafeNormal();
		const double NormalDot = FVector::DotProduct(ContactNormal, Sample->TerrainNormal.GetSafeNormal());
		if (NormalDot < Settings.MinContactNormalDot)
		{
			AddIssue(Evaluation, Candidate, TEXT("fit.contact.normal"), Contact.Name,
				FString::Printf(TEXT("contact normal alignment %.3f is below %.3f"),
					NormalDot, Settings.MinContactNormalDot),
				NormalDot, Settings.MinContactNormalDot);
		}
	}

	if (HasVolume(Candidate.Contract.ClearanceExtent))
	{
		const FBox Clearance = ContractBox(
			Candidate.Contract.BoundsCenter,
			Candidate.Contract.ClearanceExtent,
			Candidate.Contract.CentimetersPerUnit,
			Candidate.Transform);
		for (const FGatersPhysicalFitObstacle& Obstacle : Candidate.Obstacles)
		{
			if (Clearance.Intersect(Obstacle.Bounds))
			{
				AddIssue(Evaluation, Candidate, TEXT("fit.clearance.blocked"),
					Candidate.Contract.AssetId, TEXT("asset clearance is obstructed"),
					0.0, 0.0, Obstacle.Id);
			}
		}
	}

	for (const FGatersAssetPort& Port : Candidate.Contract.Ports)
	{
		if (!HasVolume(Port.ClearanceExtent))
		{
			continue;
		}
		FTransform PortTransform = Port.Transform;
		PortTransform.SetLocation(PortTransform.GetLocation() * Candidate.Contract.CentimetersPerUnit);
		const FVector Extent = Port.ClearanceExtent * Candidate.Contract.CentimetersPerUnit;
		const FBox PortClearance = FBox(-Extent, Extent)
			.TransformBy(PortTransform)
			.TransformBy(Candidate.Transform);
		for (const FGatersPhysicalFitObstacle& Obstacle : Candidate.Obstacles)
		{
			if (PortClearance.Intersect(Obstacle.Bounds))
			{
				AddIssue(Evaluation, Candidate, TEXT("fit.port.blocked"), Port.Name,
					TEXT("port clearance is obstructed"), 0.0, 0.0, Obstacle.Id);
			}
		}
	}
	return Evaluation;
}

TArray<FGatersPhysicalFitEvaluation> FGatersPhysicalFitEvaluator::EvaluateWorld(
	const FGatersCompiledWorld& World,
	const FGatersEnvironment& Environment,
	const float PadRadius,
	const FVector2D& RouteTarget,
	const FGatersPhysicalFitSettings& Settings)
{
	TArray<FGatersPhysicalFitEvaluation> Evaluations;
	for (const FGatersCompiledNode& Node : World.Nodes)
	{
		if (!Node.AssetContract.IsSet())
		{
			continue;
		}
		const FGatersPhysicalFitCandidate Candidate = SampleTerrain(
			Node.NodeId,
			Node.AssetContract.GetValue(),
			Node.Transform,
			Environment,
			PadRadius,
			RouteTarget);
		Evaluations.Add(Evaluate(Candidate, Settings));
	}
	return Evaluations;
}
