#include <catch2/catch_test_macros.hpp>
#include "NovelMind/editor/asset_pipeline.hpp"

using namespace NovelMind;
using namespace NovelMind::editor;

// =============================================================================
// AssetType Tests
// =============================================================================

TEST_CASE("AssetType - Enum values", "[asset_type]")
{
    CHECK(static_cast<u8>(AssetType::Unknown) == 0);
    CHECK(static_cast<u8>(AssetType::Image) == 1);
    CHECK(static_cast<u8>(AssetType::Audio) == 2);
    CHECK(static_cast<u8>(AssetType::Font) == 3);
}

// =============================================================================
// ImageImportSettings Tests
// =============================================================================

TEST_CASE("ImageImportSettings - Default values", "[image_import]")
{
    ImageImportSettings settings;

    CHECK(settings.compression == ImageCompression::PNG);
    CHECK(settings.generateMipmaps == false);
    CHECK(settings.premultiplyAlpha == true);
    CHECK(settings.maxWidth == 4096);
    CHECK(settings.maxHeight == 4096);
    CHECK(settings.isSpriteSheet == false);
}

TEST_CASE("ImageImportSettings - Can modify values", "[image_import]")
{
    ImageImportSettings settings;

    settings.generateMipmaps = true;
    settings.maxWidth = 2048;
    settings.maxHeight = 2048;
    settings.isSpriteSheet = true;
    settings.spriteWidth = 32;
    settings.spriteHeight = 32;

    CHECK(settings.generateMipmaps == true);
    CHECK(settings.maxWidth == 2048);
    CHECK(settings.isSpriteSheet == true);
    CHECK(settings.spriteWidth == 32);
}

// =============================================================================
// AudioImportSettings Tests
// =============================================================================

TEST_CASE("AudioImportSettings - Default values", "[audio_import]")
{
    AudioImportSettings settings;

    CHECK(settings.format == AudioFormat::OGG);
    CHECK(settings.streaming == false);
    CHECK(settings.mono == false);
    CHECK(settings.sampleRate == 44100);
    CHECK(settings.normalize == false);
}

TEST_CASE("AudioImportSettings - Can modify values", "[audio_import]")
{
    AudioImportSettings settings;

    settings.format = AudioFormat::WAV;
    settings.streaming = true;
    settings.mono = true;
    settings.sampleRate = 22050;

    CHECK(settings.format == AudioFormat::WAV);
    CHECK(settings.streaming == true);
    CHECK(settings.mono == true);
    CHECK(settings.sampleRate == 22050);
}

// =============================================================================
// ImageCompression Tests
// =============================================================================

TEST_CASE("ImageCompression - Enum values", "[image_compression]")
{
    CHECK(static_cast<u8>(ImageCompression::None) == 0);
    CHECK(static_cast<u8>(ImageCompression::DXT) == 1);
    CHECK(static_cast<u8>(ImageCompression::PNG) == 4);
}

// =============================================================================
// AudioFormat Tests
// =============================================================================

TEST_CASE("AudioFormat - Enum values", "[audio_format]")
{
    CHECK(static_cast<u8>(AudioFormat::WAV) == 0);
    CHECK(static_cast<u8>(AudioFormat::OGG) == 1);
}

// =============================================================================
// AssetDatabase Tests
// =============================================================================

TEST_CASE("AssetDatabase - Construction", "[asset_database]")
{
    AssetDatabase db;
    CHECK(true);  // Just verify construction
}

TEST_CASE("AssetDatabase - Initially empty", "[asset_database]")
{
    AssetDatabase db;

    auto assets = db.getAllAssets();
    CHECK(assets.empty());
}

TEST_CASE("AssetDatabase - Add and retrieve asset", "[asset_database]")
{
    AssetDatabase db;

    AssetEntry entry;
    entry.id = "test_asset";
    entry.name = "Test Asset";
    entry.type = AssetType::Image;
    entry.sourcePath = "/path/to/image.png";

    db.addAsset(entry);

    auto retrieved = db.getAsset("test_asset");
    REQUIRE(retrieved.has_value());
    CHECK(retrieved->name == "Test Asset");
    CHECK(retrieved->type == AssetType::Image);
}

TEST_CASE("AssetDatabase - Has asset check", "[asset_database]")
{
    AssetDatabase db;

    CHECK(db.hasAsset("nonexistent") == false);

    AssetEntry entry;
    entry.id = "exists";
    entry.type = AssetType::Audio;
    db.addAsset(entry);

    CHECK(db.hasAsset("exists") == true);
}

TEST_CASE("AssetDatabase - Remove asset", "[asset_database]")
{
    AssetDatabase db;

    AssetEntry entry;
    entry.id = "to_remove";
    entry.type = AssetType::Font;
    db.addAsset(entry);

    CHECK(db.hasAsset("to_remove") == true);

    db.removeAsset("to_remove");

    CHECK(db.hasAsset("to_remove") == false);
}

TEST_CASE("AssetDatabase - Clear all assets", "[asset_database]")
{
    AssetDatabase db;

    AssetEntry entry1;
    entry1.id = "asset1";
    entry1.type = AssetType::Image;
    db.addAsset(entry1);

    AssetEntry entry2;
    entry2.id = "asset2";
    entry2.type = AssetType::Audio;
    db.addAsset(entry2);

    CHECK(db.getAllAssets().size() == 2);

    db.clear();

    CHECK(db.getAllAssets().empty());
}

TEST_CASE("AssetDatabase - Get assets by type", "[asset_database]")
{
    AssetDatabase db;

    AssetEntry img1;
    img1.id = "img1";
    img1.type = AssetType::Image;
    db.addAsset(img1);

    AssetEntry img2;
    img2.id = "img2";
    img2.type = AssetType::Image;
    db.addAsset(img2);

    AssetEntry audio;
    audio.id = "audio1";
    audio.type = AssetType::Audio;
    db.addAsset(audio);

    auto images = db.getAssetsByType(AssetType::Image);
    CHECK(images.size() == 2);

    auto audios = db.getAssetsByType(AssetType::Audio);
    CHECK(audios.size() == 1);
}

// =============================================================================
// ImageImporter Tests
// =============================================================================

TEST_CASE("ImageImporter - Construction", "[image_importer]")
{
    ImageImporter importer;
    CHECK(true);
}

TEST_CASE("ImageImporter - Can import PNG", "[image_importer]")
{
    ImageImporter importer;

    CHECK(importer.canImport("image.png") == true);
    CHECK(importer.canImport("image.PNG") == true);
    CHECK(importer.canImport("/path/to/image.png") == true);
}

TEST_CASE("ImageImporter - Can import JPEG", "[image_importer]")
{
    ImageImporter importer;

    CHECK(importer.canImport("image.jpg") == true);
    CHECK(importer.canImport("image.jpeg") == true);
}

TEST_CASE("ImageImporter - Cannot import non-image", "[image_importer]")
{
    ImageImporter importer;

    CHECK(importer.canImport("audio.wav") == false);
    CHECK(importer.canImport("script.nms") == false);
    CHECK(importer.canImport("font.ttf") == false);
}

// =============================================================================
// AudioImporter Tests
// =============================================================================

TEST_CASE("AudioImporter - Construction", "[audio_importer]")
{
    AudioImporter importer;
    CHECK(true);
}

TEST_CASE("AudioImporter - Can import WAV", "[audio_importer]")
{
    AudioImporter importer;

    CHECK(importer.canImport("audio.wav") == true);
    CHECK(importer.canImport("audio.WAV") == true);
}

TEST_CASE("AudioImporter - Can import OGG", "[audio_importer]")
{
    AudioImporter importer;

    CHECK(importer.canImport("audio.ogg") == true);
}

TEST_CASE("AudioImporter - Cannot import non-audio", "[audio_importer]")
{
    AudioImporter importer;

    CHECK(importer.canImport("image.png") == false);
    CHECK(importer.canImport("font.ttf") == false);
}

// =============================================================================
// FontImporter Tests
// =============================================================================

TEST_CASE("FontImporter - Construction", "[font_importer]")
{
    FontImporter importer;
    CHECK(true);
}

TEST_CASE("FontImporter - Can import TTF", "[font_importer]")
{
    FontImporter importer;

    CHECK(importer.canImport("font.ttf") == true);
    CHECK(importer.canImport("font.TTF") == true);
}

TEST_CASE("FontImporter - Can import OTF", "[font_importer]")
{
    FontImporter importer;

    CHECK(importer.canImport("font.otf") == true);
}

TEST_CASE("FontImporter - Cannot import non-font", "[font_importer]")
{
    FontImporter importer;

    CHECK(importer.canImport("image.png") == false);
    CHECK(importer.canImport("audio.wav") == false);
}
