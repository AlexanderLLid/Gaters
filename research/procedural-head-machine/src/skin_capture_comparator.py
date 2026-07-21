import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont


def compare(champion, challenger, recipe):
    champion_ratio = champion["metrics"]["elbow_thickness_ratio_p10"]
    challenger_ratio = challenger["metrics"]["elbow_thickness_ratio_p10"]
    improvement = challenger_ratio - champion_ratio
    failures = []
    if not champion.get("passed") or not challenger.get("passed"):
        failures.append("candidate-verification")
    if challenger_ratio < recipe["minimum_thickness_ratio_p10"]:
        failures.append("thickness-threshold")
    if improvement < recipe["minimum_improvement"]:
        failures.append("insufficient-improvement")
    if challenger["metrics"]["max_influences"] > recipe["maximum_influences"]:
        failures.append("influence-budget")
    return {
        "schema": "skin-capture-comparison/0",
        "promote_challenger": not failures,
        "failures": failures,
        "metrics": {
            "champion_thickness_ratio_p10": champion_ratio,
            "challenger_thickness_ratio_p10": challenger_ratio,
            "improvement": improvement,
            "challenger_max_influences": challenger["metrics"]["max_influences"],
        },
    }


def _font(size):
    try:
        return ImageFont.truetype("arial.ttf", size)
    except OSError:
        return ImageFont.load_default()


def render(champion_run, challenger_run, report, output):
    champion = Image.open(champion_run / "pose-preview.png").convert("RGB").crop((50, 180, 785, 840))
    challenger = Image.open(challenger_run / "pose-preview.png").convert("RGB").crop((50, 180, 785, 840))
    image = Image.new("RGB", (1600, 900), (22, 24, 27))
    draw = ImageDraw.Draw(image)
    draw.text((50, 28), "ELBOW CAPTURE COMPARISON - SAME MESH, SKELETON AND POSE", fill=(244, 245, 247), font=_font(29))
    metrics = report["metrics"]
    draw.text((50, 70), f"A proximity {metrics['champion_thickness_ratio_p10']:.1%} retained  |  B biharmonic {metrics['challenger_thickness_ratio_p10']:.1%} retained  |  +{metrics['improvement']:.1%}", fill=(171, 178, 188), font=_font(19))
    draw.text((72, 125), "A - PROXIMITY", fill=(245, 246, 248), font=_font(25))
    draw.text((837, 125), "B - BIHARMONIC (4 INFLUENCES)", fill=(245, 246, 248), font=_font(25))
    image.paste(champion, (50, 170))
    image.paste(challenger, (815, 170))
    image.save(output)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("champion_run", type=Path)
    parser.add_argument("challenger_run", type=Path)
    parser.add_argument("recipe", type=Path)
    parser.add_argument("output_root", type=Path)
    args = parser.parse_args()
    champion = json.loads((args.champion_run / "verification.json").read_text(encoding="utf-8"))
    challenger = json.loads((args.challenger_run / "verification.json").read_text(encoding="utf-8"))
    recipe = json.loads(args.recipe.read_text(encoding="utf-8"))
    report = compare(champion, challenger, recipe)
    args.output_root.mkdir(parents=True, exist_ok=False)
    report["champion_run"] = str(args.champion_run)
    report["challenger_run"] = str(args.challenger_run)
    report["recipe"] = recipe
    (args.output_root / "comparison.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    render(args.champion_run, args.challenger_run, report, args.output_root / "preview.png")
    print(f"SKIN_CAPTURE_COMPARISON_{'PROMOTE' if report['promote_challenger'] else 'REJECT'} root={args.output_root}")
    raise SystemExit(0 if report["promote_challenger"] else 1)


if __name__ == "__main__":
    main()
