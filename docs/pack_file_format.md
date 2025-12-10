# Pack File Format Specification

This document specifies the binary format for NovelMind resource pack files (`.nmres`).

## Overview

NovelMind uses a custom pack file format to store encrypted game resources. The format is designed to:

- Protect assets from casual extraction
- Support fast random access to individual resources
- Allow streaming of large resources
- Enable integrity verification

## File Structure

```
+------------------+
|     Header       |  (64 bytes)
+------------------+
|   Resource Table |  (variable)
+------------------+
|   String Table   |  (variable)
+------------------+
|   Resource Data  |  (variable)
+------------------+
|     Footer       |  (32 bytes)
+------------------+
```

## Header Format (64 bytes)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0x00 | 4 | char[4] | Magic number: "NMRS" |
| 0x04 | 2 | uint16 | Format version (major) |
| 0x06 | 2 | uint16 | Format version (minor) |
| 0x08 | 4 | uint32 | Flags |
| 0x0C | 4 | uint32 | Resource count |
| 0x10 | 8 | uint64 | Resource table offset |
| 0x18 | 8 | uint64 | String table offset |
| 0x20 | 8 | uint64 | Data section offset |
| 0x28 | 8 | uint64 | Total file size |
| 0x30 | 16 | uint8[16] | Content hash (first 128 bits of SHA-256) |

### Flags (bit field)

| Bit | Name | Description |
|-----|------|-------------|
| 0 | ENCRYPTED | Resources are encrypted |
| 1 | COMPRESSED | Resources are compressed |
| 2 | SIGNED | Pack includes digital signature |
| 3-31 | Reserved | Must be zero |

## Resource Table Entry (48 bytes each)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0x00 | 4 | uint32 | Resource ID string offset |
| 0x04 | 4 | uint32 | Resource type |
| 0x08 | 8 | uint64 | Data offset (from data section start) |
| 0x10 | 8 | uint64 | Compressed size |
| 0x18 | 8 | uint64 | Uncompressed size |
| 0x20 | 4 | uint32 | Flags |
| 0x24 | 4 | uint32 | Checksum (CRC32) |
| 0x28 | 8 | uint8[8] | Initialization vector (for encryption) |

### Resource Types

| Value | Type | Description |
|-------|------|-------------|
| 0x00 | Unknown | Unspecified type |
| 0x01 | Texture | Image data (PNG, etc.) |
| 0x02 | Audio | Sound effect |
| 0x03 | Music | Background music (streaming) |
| 0x04 | Font | Font file |
| 0x05 | Script | Compiled NM Script bytecode |
| 0x06 | Scene | Scene definition |
| 0x07 | Localization | Translation strings |
| 0x08 | Data | Generic data blob |

### Resource Flags

| Bit | Name | Description |
|-----|------|-------------|
| 0 | STREAMABLE | Resource should be streamed |
| 1 | PRELOAD | Resource should be preloaded |
| 2-31 | Reserved | Must be zero |

## String Table

The string table stores all resource ID strings in a contiguous block.

```
+------------------+
|  String Count    |  (4 bytes, uint32)
+------------------+
|  Offset Table    |  (4 bytes * count)
+------------------+
|   String Data    |  (null-terminated UTF-8 strings)
+------------------+
```

Resource IDs are stored as null-terminated UTF-8 strings. The offset table contains offsets from the start of the string data section.

## Resource Data Section

Resources are stored sequentially with optional alignment padding. Each resource's data is:

1. Optionally compressed (zlib/LZ4)
2. Encrypted (AES-256-GCM)
3. Stored at the offset specified in the resource table

### Alignment

- Resources larger than 4KB are aligned to 4KB boundaries
- Smaller resources are aligned to 16-byte boundaries
- Alignment enables memory-mapped access and streaming

## Footer (32 bytes)

| Offset | Size | Type | Description |
|--------|------|------|-------------|
| 0x00 | 4 | uint32 | Footer magic: "NMRF" |
| 0x04 | 4 | uint32 | CRC32 of header + resource table + string table |
| 0x08 | 8 | uint64 | Pack creation timestamp (Unix epoch) |
| 0x10 | 4 | uint32 | Build number |
| 0x14 | 12 | uint8[12] | Reserved (must be zero) |

## Encryption

### Key Derivation

The encryption key is derived from a project-specific master key:

```
salt = SHA256(project_id + build_timestamp)
key = PBKDF2-SHA256(master_key, salt, iterations=100000, key_length=32)
```

### Resource Encryption

Each resource is encrypted individually using AES-256-GCM:

- Unique IV per resource (stored in resource table)
- Authentication tag appended to encrypted data (16 bytes)
- Associated data: resource ID + type + size

### Key Storage

The derived key is obfuscated and embedded in the executable:

1. Split key into multiple fragments
2. XOR with code section hashes
3. Store fragments in different locations
4. Reconstruct at runtime

Note: This provides protection against casual extraction but is not cryptographically secure against determined reverse engineering.

## Compression

Resources can be compressed before encryption using:

- **LZ4** (default): Fast decompression, moderate compression
- **zlib**: Better compression, slower decompression

The compression algorithm is indicated in the pack flags. Individual resources may be stored uncompressed if compression provides no benefit (e.g., already compressed images).

## Pack Building Process

```
1. Collect all resources
2. For each resource:
   a. Read source file
   b. Generate resource ID (hash of original path)
   c. Compress if beneficial
   d. Generate unique IV
   e. Encrypt with AES-256-GCM
   f. Calculate CRC32
3. Build resource table
4. Build string table
5. Write header (placeholder hashes)
6. Write resource table
7. Write string table
8. Write resource data
9. Calculate content hash
10. Write footer
11. Update header with final hash
```

## Runtime Loading Process

```
1. Read and validate header
2. Read footer and verify table CRC
3. Load resource table into memory
4. Load string table into memory
5. Build resource ID -> entry map
6. On resource request:
   a. Look up entry by ID
   b. Read encrypted data from offset
   c. Decrypt with resource IV
   d. Decompress if needed
   e. Verify CRC32
   f. Return decrypted data
```

## Example Resource Layout

```
Header (64 bytes)
  Magic: "NMRS"
  Version: 1.0
  Flags: ENCRYPTED | COMPRESSED
  Resource Count: 3
  ...

Resource Table (144 bytes = 3 * 48)
  [0] ID: "bg_city" | Type: Texture | Offset: 0 | Size: 245760
  [1] ID: "bgm_main" | Type: Music | Offset: 249856 | Size: 1048576
  [2] ID: "scene_intro" | Type: Script | Offset: 1298432 | Size: 4096

String Table
  Count: 3
  Offsets: [0, 8, 17]
  Data: "bg_city\0bgm_main\0scene_intro\0"

Resource Data
  [0x000000] bg_city texture data (encrypted, 245760 bytes)
  [0x03D000] bgm_main music data (encrypted, 1048576 bytes)
  [0x13D000] scene_intro script data (encrypted, 4096 bytes)

Footer (32 bytes)
  ...
```

## Versioning

| Version | Changes |
|---------|---------|
| 1.0 | Initial format |

Future versions will maintain backward compatibility where possible. The runtime will refuse to load packs with incompatible major versions.

## Security Considerations

The pack file format provides protection against:

- Direct file extraction from game directory
- Casual modification of game resources
- Tampering detection via checksums

The format does NOT provide protection against:

- Determined reverse engineering
- Memory dumping during runtime
- Executable patching

For sensitive content, consider additional legal protections (DMCA, etc.) alongside technical measures.
