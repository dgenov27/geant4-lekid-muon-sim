#include "RunAction.hh"
#include "GeometryConfig.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Material.hh"
#include "G4ThreeVector.hh"
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <filesystem>


static std::ofstream& UncStream() {
    static std::ofstream ofs;
    if (!ofs.is_open()) {
        ofs.open("l2_uncertainty.csv", std::ios::trunc);
        ofs << "event,L2_pred_pixcenter_x_mm,L2_pred_pixcenter_y_mm,i_pred,j_pred,"
            "sigma_x_mm,sigma_y_mm,sigma_r_mm,r95_mm,sigma_r_px,r95_px,crossesAl\n";

    }
    return ofs;
}

 
static inline G4ThreeVector PredictL2Pixelized(const G4ThreeVector& p1, double z1,
    const G4ThreeVector& p3, double z3,
    double z2,
    double chipXY_mm, int nx, int ny) {
    double pitch_mm = chipXY_mm / nx;

    auto snapToPixel = [&](double coord_mm) {
        // map to pixel center: find bin via floor, then shift +0.5 to center
        int ix = static_cast<int>(std::floor(coord_mm / pitch_mm));
        return (ix + 0.5) * pitch_mm;
        };

    // Pixelize x,y of L1 and L3
    double x1_pix = snapToPixel(p1.x() / mm) * mm;
    double y1_pix = snapToPixel(p1.y() / mm) * mm;
    double x3_pix = snapToPixel(p3.x() / mm) * mm;
    double y3_pix = snapToPixel(p3.y() / mm) * mm;

    // Straight line extrapolation using pixelized positions
    const double t = (z2 - z1) / (z3 - z1);
    return G4ThreeVector(
        x1_pix + (x3_pix - x1_pix) * t,
        y1_pix + (y3_pix - y1_pix) * t,
        z2
    );
}


RunAction::RunAction() : G4UserRunAction() {}
RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*) {
    totalEdep = 0.0;
    eventHits.clear();
    G4cout << ">>> BeginOfRunAction CALLED" << G4endl;
}


void RunAction::EndOfRunAction(const G4Run*) {
    G4cout << ">>> EndOfRunAction CALLED" << G4endl;
    G4cout << "Total energy deposited: " << G4BestUnit(totalEdep, "Energy") << G4endl;
    G4cout << "[RunAction] CWD = " << std::filesystem::current_path().string() << G4endl;


    std::ofstream out("deflection_results.csv");
    out << "eventID,l1_x_mm,l1_y_mm,l1_z_mm,l3_x_mm,l3_y_mm,l3_z_mm,"
        "pred_x_mm,pred_y_mm,pred_z_mm,act2_x_mm,act2_y_mm,act2_z_mm,err_mm,"
        "px1,py1,px3,py3,pixel_pred_x,pixel_pred_y,pixel_act_x,pixel_act_y,"
        "px1_mm,py1_mm,px3_mm,py3_mm,pixel_pred_x_mm,pixel_pred_y_mm,pixel_act_x_mm,pixel_act_y_mm\n";



    out << std::fixed << std::setprecision(6);

    const auto& fs1 = gStack.layer[0];
    const auto& fs2 = gStack.layer[1];
    double chipXY_mm = fs1.chipXY / mm;
    int nx = 200, ny = 200; // pixel grid (coarser 200x200 or future 1000x1000)
    double pitch_mm = chipXY_mm / nx;

    // Substrate centers used in DetectorConstruction
    double z1c = 0.0;
    double z2c = z1c + fs1.siThickness + gStack.gap12;
    double z3c = z2c + fs2.siThickness + gStack.gap23;

    // --- L2 entry plane INCLUDING films (NbTiN, Al, Al2O3, optional SiN) ---
    auto entryZ = [&](const FilmStack& fs, double zc) {
        // start at top of Si
        double z = zc - fs.siThickness * 0.5;
        // then add any films that sit above Si (enabled only)
        if (fs.use_nbtiN && fs.nbtiN_thick > 0) z += fs.nbtiN_thick;
        if (fs.use_al && fs.al_thick > 0) z += fs.al_thick;
        if (fs.use_al2o3 && fs.al2o3_thick > 0) z += fs.al2o3_thick;
        if (fs.use_sin && fs.sin_thick > 0) z += fs.sin_thick;
        return z;
        };
    double z2_plane = entryZ(fs2, z2c);   // TOP of the full L2 stack


    for (const auto& evPair : eventHits) {
        long eventID = evPair.first;
        const auto& layerMap = evPair.second;

        auto it1 = layerMap.find("Layer1");
        auto it3 = layerMap.find("Layer3");
        bool hasL1 = (it1 != layerMap.end() && it1->second.has);
        bool hasL3 = (it3 != layerMap.end() && it3->second.has);
        auto it2 = layerMap.find("Layer2");
        bool hasAct2 = (it2 != layerMap.end() && it2->second.has);
        if (!hasL1 || !hasL3) continue;

        G4ThreeVector p1 = it1->second.pos;
        G4ThreeVector p3 = it3->second.pos;

        // Use the actual SURFACE z of the recorded hits for L1 and L3
        const double z1_surf = p1.z();
        const double z3_surf = p3.z();

        // Pixel-based straight-line L2 prediction (still pixel-only x,y)
        G4ThreeVector pred2 = PredictL2Pixelized(p1, z1_surf, p3, z3_surf, z2_plane, chipXY_mm, nx, ny);



        // MS uncertainty from L1 stack (Si + full NbTiN + conditional Al/ox/SiN if hit footprint)
        G4double t_rad = 0.0;
        t_rad += fs1.siThickness / G4Material::GetMaterial("G4_Si")->GetRadlen();
        if (fs1.use_nbtiN && fs1.nbtiN_thick > 0)
            t_rad += fs1.nbtiN_thick / G4Material::GetMaterial("NbTiNApprox")->GetRadlen();
        bool crossesAl = fs1.use_al && std::abs(p1.x()) <= fs1.al_width * 0.5 && std::abs(p1.y()) <= fs1.al_length * 0.5;
        if (crossesAl) {
            t_rad += fs1.al_thick / G4Material::GetMaterial("G4_Al")->GetRadlen();
            if (fs1.use_al2o3 && fs1.al2o3_thick > 0)
                t_rad += fs1.al2o3_thick / G4Material::GetMaterial("Al2O3_custom")->GetRadlen();
            if (fs1.use_sin && fs1.sin_thick > 0)
                t_rad += fs1.sin_thick / G4Material::GetMaterial("Si3N4")->GetRadlen();
        }

        G4double theta0 = HighlandTheta0(gBeam.p_MeV, gBeam.beta, t_rad);
        // True lever arm from the L1 hit surface to the L2 entry plane
        G4double L = std::abs(z2_plane - z1_surf);
        G4double sigma_x = L * theta0;
        G4double sigma_y = L * theta0;
        G4double sigma_r = std::sqrt(2.0) * sigma_x;
        G4double r95 = sigma_x * std::sqrt(-2.0 * std::log(1.0 - 0.95));

        auto toPixel = [&](const G4ThreeVector& p) {
            double x_mm = p.x() / mm + 0.5 * chipXY_mm;
            double y_mm = p.y() / mm + 0.5 * chipXY_mm;
            int ix = std::clamp(int(std::floor(x_mm / pitch_mm)), 0, nx - 1);
            int iy = std::clamp(int(std::floor(y_mm / pitch_mm)), 0, ny - 1);
            return std::pair<int, int>(ix, iy);
            };

        auto [i_pred, j_pred] = toPixel(pred2);
        double sigma_r_px = (sigma_r / mm) / pitch_mm;
        double r95_px = (r95 / mm) / pitch_mm;

        G4ThreeVector act2(0, 0, 0);
        if (hasAct2) act2 = it2->second.pos;
        double err = (hasAct2 ? (pred2 - act2).mag() / mm : -1.0);

        auto [px1_i, py1_i] = toPixel(p1);
        auto [px3_i, py3_i] = toPixel(p3);
        auto [ppx_i, ppy_i] = toPixel(pred2);
        auto [pa2_i, pya2_i] = toPixel(act2);


        // Convert back to pixelized mm coordinates (center of pixel)
        auto toPixelMM = [&](int ix, int iy) {
            double x_mm = (ix + 0.5) * pitch_mm - 0.5 * chipXY_mm;
            double y_mm = (iy + 0.5) * pitch_mm - 0.5 * chipXY_mm;
            return std::pair<double, double>(x_mm, y_mm);
            };

        auto [px1_mm, py1_mm] = toPixelMM(px1_i, py1_i);
        auto [px3_mm, py3_mm] = toPixelMM(px3_i, py3_i);
        auto [ppx_mm, ppy_mm] = toPixelMM(ppx_i, ppy_i);
        auto [pa2_mm, pya2_mm] = toPixelMM(pa2_i, pya2_i);


        out << eventID << ','
            << p1.x() / mm << ',' << p1.y() / mm << ',' << p1.z() / mm << ','
            << p3.x() / mm << ',' << p3.y() / mm << ',' << p3.z() / mm << ','
            << pred2.x() / mm << ',' << pred2.y() / mm << ',' << z2_plane / mm << ','
            << act2.x() / mm << ',' << act2.y() / mm << ',' << act2.z() / mm << ','
            << err << ','
            << px1_i << ',' << py1_i << ',' << px3_i << ',' << py3_i << ','  // NEW: L3 pixel indices
            << ppx_i << ',' << ppy_i << ',' << pa2_i << ',' << pya2_i << ','
            << px1_mm << ',' << py1_mm << ',' << px3_mm << ',' << py3_mm << ',' // NEW: L3 pixel centers (mm)
            << ppx_mm << ',' << ppy_mm << ','
            << pa2_mm << ',' << pya2_mm << '\n';




        auto& ufs = UncStream();
        ufs << eventID << ','
            << ppx_mm << ',' << ppy_mm << ','   // pixelized predicted coords in mm
            << i_pred << ',' << j_pred << ','   // pixel indices
            << sigma_x / mm << ',' << sigma_y / mm << ','
            << sigma_r / mm << ',' << r95 / mm << ','
            << sigma_r_px << ',' << r95_px << ','
            << (crossesAl ? 1 : 0) << '\n';

    }

    out.close();

    UncStream().flush();
    UncStream().close();

    G4cout << "Wrote deflection_results.csv and l2_uncertainty.csv for " << eventHits.size() << " events." << G4endl;
}



void RunAction::SetLayerHit(long eventID, const std::string& layer, const G4ThreeVector& pos) {
    auto& slot = eventHits[eventID][layer];
    if (!slot.has) { slot.has = true; slot.pos = pos; } // first crossing only
}
void RunAction::RecordLayer2Actual(long eventID, const G4ThreeVector& pos) { SetLayerHit(eventID, "Layer2", pos); }
