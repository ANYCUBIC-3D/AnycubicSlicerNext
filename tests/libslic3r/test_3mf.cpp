#include <catch2/catch.hpp>

#include "libslic3r/Model.hpp"
#include "libslic3r/Preset.hpp"
#include "libslic3r/Format/3mf.hpp"
#include "libslic3r/Format/bbs_3mf.hpp"
#include "libslic3r/Format/STL.hpp"
#include "libslic3r/Zipper.hpp"

#include <boost/filesystem/operations.hpp>
#include <boost/nowide/fstream.hpp>
#include <iterator>

using namespace Slic3r;

namespace {

struct Sanitizer3mfFixture
{
    boost::filesystem::path directory =
        boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("sanitizer-%%%%-%%%%-%%%%");
    boost::filesystem::path path = directory / "project.3mf";
    Model model;
    DynamicPrintConfig config;
    ConfigSubstitutionContext substitutions{ForwardCompatibilitySubstitutionRule::Disable};
    PlateDataPtrs plates;
    std::vector<Preset *> presets;

    Sanitizer3mfFixture()
    {
        boost::filesystem::create_directory(directory);
        model.set_backup_path((directory / "backup").string());
    }

    ~Sanitizer3mfFixture()
    {
        release_PlateData_list(plates);
        for (Preset *preset : presets)
            delete preset;
        model.set_backup_path("detach");
        boost::system::error_code error;
        boost::filesystem::remove_all(directory, error);
        if (error)
            WARN("Could not remove the temporary 3MF fixture: " << error.message());
    }

    void write(const std::string &settings, const std::string &preset = {})
    {
        Zipper archive(path.string());
        archive.add_entry("[Content_Types].xml");
        archive << R"(<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
<Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/>
</Types>)";
        archive.add_entry("_rels/.rels");
        archive << R"(<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
<Relationship Id="rel0" Target="/3D/3dmodel.model" Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel"/>
</Relationships>)";
        archive.add_entry("3D/3dmodel.model");
        archive << R"(<model unit="millimeter" xmlns="http://schemas.microsoft.com/3dmanufacturing/core/2015/02">
<metadata name="BambuStudio:3mfVersion">1</metadata>
<resources><object id="1" type="model"><mesh>
<vertices>
<vertex x="0" y="0" z="0"/><vertex x="10" y="0" z="0"/>
<vertex x="0" y="10" z="0"/><vertex x="0" y="0" z="10"/>
</vertices>
<triangles>
<triangle v1="0" v2="2" v3="1"/><triangle v1="0" v2="1" v3="3"/>
<triangle v1="0" v2="3" v3="2"/><triangle v1="1" v2="2" v3="3"/>
</triangles>
</mesh></object></resources>
<build><item objectid="1"/></build>
</model>)";
        archive.add_entry("Metadata/project_settings.config");
        archive << settings;
        if (!preset.empty()) {
            archive.add_entry("Metadata/process_settings_1.config");
            archive << preset;
        }
        archive.finalize();
    }

    bool load(LoadStrategy strategy)
    {
        bool from_bbs = false;
        Semver version;
        return load_bbs_3mf(path.string().c_str(), &config, &substitutions, &model, &plates, &presets,
                            &from_bbs, &version, nullptr, strategy);
    }

    std::string contents() const
    {
        boost::nowide::ifstream input(path.string(), std::ios::binary);
        REQUIRE(input.good());
        return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    }
};

} // namespace

TEST_CASE("3MF sanitization requires consent for each import", "[3mf][sanitizer]")
{
    const std::string settings = R"({
        "print_settings_id": "Sanitizer test",
        "raft_first_layer_expansion": "-1",
        "tree_support_wall_count": "-1",
        "wall_loops": "3"
    })";

    for (bool sanitize : {true, false}) {
        CAPTURE(sanitize);
        Sanitizer3mfFixture file;
        file.write(settings, settings);
        const std::string original = file.contents();
        LoadStrategy strategy = LoadStrategy::LoadModel | LoadStrategy::LoadConfig;
        if (sanitize)
            strategy = strategy | LoadStrategy::SanitizeConfig;
        REQUIRE(file.load(strategy));
        REQUIRE(file.model.objects.size() == 1);
        CHECK(file.config.opt_float("raft_first_layer_expansion") == (sanitize ? 0 : -1));
        CHECK(file.config.opt_int("tree_support_wall_count") == (sanitize ? 0 : -1));
        CHECK(file.config.opt_int("wall_loops") == 3);
        REQUIRE(file.presets.size() == 1);
        CHECK(file.presets.front()->config.opt_float("raft_first_layer_expansion") == (sanitize ? 0 : -1));
        CHECK(file.presets.front()->config.opt_int("tree_support_wall_count") == (sanitize ? 0 : -1));
        CHECK(file.contents() == original);
    }
}

TEST_CASE("3MF sanitization handles numeric settings without changing valid values", "[3mf][sanitizer]")
{
    Sanitizer3mfFixture file;
    SECTION("Numeric minus one values")
    {
        file.write(R"({"raft_first_layer_expansion": -1.0, "tree_support_wall_count": -1, "wall_loops": "3"})");
        REQUIRE(file.load(LoadStrategy::LoadConfig | LoadStrategy::SanitizeConfig));
        CHECK(file.config.opt_float("raft_first_layer_expansion") == 0);
        CHECK(file.config.opt_int("tree_support_wall_count") == 0);
    }
    SECTION("Already valid values")
    {
        file.write(R"({"raft_first_layer_expansion": "2.5", "tree_support_wall_count": "1", "wall_loops": "3"})");
        REQUIRE(file.load(LoadStrategy::LoadConfig | LoadStrategy::SanitizeConfig));
        CHECK(file.config.opt_float("raft_first_layer_expansion") == 2.5);
        CHECK(file.config.opt_int("tree_support_wall_count") == 1);
    }
    CHECK(file.config.opt_int("wall_loops") == 3);
}

TEST_CASE("3MF sanitization rejects malformed settings", "[3mf][sanitizer]")
{
    Sanitizer3mfFixture file;
    file.write("{");
    const std::string original = file.contents();
    REQUIRE_THROWS_AS(file.load(LoadStrategy::LoadConfig | LoadStrategy::SanitizeConfig), FileIOError);
    CHECK(file.contents() == original);
}

SCENARIO("Reading 3mf file", "[3mf]") {
    GIVEN("umlauts in the path of the file") {
        Model model;
        WHEN("3mf model is read") {
        	std::string path = std::string(TEST_DATA_DIR) + "/test_3mf/Geräte/Büchse.3mf";
        	DynamicPrintConfig config;
            ConfigSubstitutionContext ctxt{ ForwardCompatibilitySubstitutionRule::Disable };
            bool ret = load_3mf(path.c_str(), config, ctxt, &model, false);
            THEN("load should succeed") {
                REQUIRE(ret);
            }
        }
    }
}

SCENARIO("Export+Import geometry to/from 3mf file cycle", "[3mf]") {
    GIVEN("world vertices coordinates before save") {
        // load a model from stl file
        Model src_model;
        std::string src_file = std::string(TEST_DATA_DIR) + "/test_3mf/Prusa.stl";
        load_stl(src_file.c_str(), &src_model);
        src_model.add_default_instances();

        ModelObject* src_object = src_model.objects.front();

        // apply generic transformation to the 1st volume
        Geometry::Transformation src_volume_transform;
        src_volume_transform.set_offset({ 10.0, 20.0, 0.0 });
        src_volume_transform.set_rotation({ Geometry::deg2rad(25.0), Geometry::deg2rad(35.0), Geometry::deg2rad(45.0) });
        src_volume_transform.set_scaling_factor({ 1.1, 1.2, 1.3 });
        src_volume_transform.set_mirror({ -1.0, 1.0, -1.0 });
        src_object->volumes.front()->set_transformation(src_volume_transform);

        // apply generic transformation to the 1st instance
        Geometry::Transformation src_instance_transform;
        src_instance_transform.set_offset({ 5.0, 10.0, 0.0 });
        src_instance_transform.set_rotation({ Geometry::deg2rad(12.0), Geometry::deg2rad(13.0), Geometry::deg2rad(14.0) });
        src_instance_transform.set_scaling_factor({ 0.9, 0.8, 0.7 });
        src_instance_transform.set_mirror({ 1.0, -1.0, -1.0 });
        src_object->instances.front()->set_transformation(src_instance_transform);

        WHEN("model is saved+loaded to/from 3mf file") {
            // save the model to 3mf file
            std::string test_file = std::string(TEST_DATA_DIR) + "/test_3mf/prusa.3mf";
            store_3mf(test_file.c_str(), &src_model, nullptr, false);

            // load back the model from the 3mf file
            Model dst_model;
            DynamicPrintConfig dst_config;
            {
                ConfigSubstitutionContext ctxt{ ForwardCompatibilitySubstitutionRule::Disable };
                load_3mf(test_file.c_str(), dst_config, ctxt, &dst_model, false);
            }
            boost::filesystem::remove(test_file);

            // compare meshes
            TriangleMesh src_mesh = src_model.mesh();
            TriangleMesh dst_mesh = dst_model.mesh();

            bool res = src_mesh.its.vertices.size() == dst_mesh.its.vertices.size();
            if (res) {
                for (size_t i = 0; i < dst_mesh.its.vertices.size(); ++i) {
                    res &= dst_mesh.its.vertices[i].isApprox(src_mesh.its.vertices[i]);
                }
            }
            THEN("world vertices coordinates after load match") {
                REQUIRE(res);
            }
        }
    }
}

SCENARIO("2D convex hull of sinking object", "[3mf]") {
    GIVEN("model") {
        // load a model
        Model model;
        std::string src_file = std::string(TEST_DATA_DIR) + "/test_3mf/Prusa.stl";
        load_stl(src_file.c_str(), &model);
        model.add_default_instances();

        WHEN("model is rotated, scaled and set as sinking") {
            ModelObject* object = model.objects.front();
            object->center_around_origin(false);

            // set instance's attitude so that it is rotated, scaled and sinking
            ModelInstance* instance = object->instances.front();
            instance->set_rotation(X, -M_PI / 4.0);
            instance->set_offset(Vec3d::Zero());
            instance->set_scaling_factor({ 2.0, 2.0, 2.0 });

            // calculate 2D convex hull
            Polygon hull_2d = object->convex_hull_2d(instance->get_transformation().get_matrix());

            // verify result
            Points result = {
                { -91501496, -15914144 },
                { 91501496, -15914144 },
                { 91501496, 4243 },
                { 78229680, 4246883 },
                { 56898100, 4246883 },
                { -85501496, 4242641 },
                { -91501496, 4243 }
            };

            // Allow 1um error due to floating point rounding.
            bool res = hull_2d.points.size() == result.size();
            if (res)
                for (size_t i = 0; i < result.size(); ++ i) {
                    const Point &p1 = result[i];
                    const Point &p2 = hull_2d.points[i];
                    if (std::abs(p1.x() - p2.x()) > 1 || std::abs(p1.y() - p2.y()) > 1) {
                        res = false;
                        break;
                    }
                }

            THEN("2D convex hull should match with reference") {
                REQUIRE(res);
            }
        }
    }
}
