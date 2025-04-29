# GitHub Workflows Documentation

This document describes the GitHub Actions workflows used in this project.

## CI Workflow (`ci.yml`)

**Purpose**: Continuous Integration workflow that runs on every push to dev branch and pull requests to main.

**Triggers**:
- Pull requests to `main` branch

**What it does**:
1. Sets up build environment
   - Uses Ubuntu latest
   - Configures Python
   - Sets up CMake
2. Configures vcpkg
   - Uses specific commit: efb1e7436979a30c4d3e5ab2375fd8e2e461d541
   - Caches vcpkg dependencies
3. Builds the project
   - Uses CMake presets
   - Builds in Release mode
   - Enables testing
4. Runs tests
   - Executes all test cases
   - Shows output on failure
5. Creates artifacts
   - Packages the build
   - Uploads build artifacts

**Environment Variables**:
- `MQTT_SERVER`: MQTT broker address (default: tcp://broker.emqx.io:1883)
- `MQTT_TOPIC`: MQTT topic for testing (default: test/topic)
- `MQTT_QOS`: Quality of Service level (default: 1)
- `MQTT_CLIENT_ID`: Client identifier (default: test_client)

**Cache Strategy**:
- Caches CMake build files
- Caches vcpkg dependencies
- Uses hash of CMake files for cache key

## Version Update Workflow (`version-update.yml`)

**Purpose**: Automatically updates project version when pull requests are merged.

**Triggers**:
- Pull request closed (merged) to `main` branch

**What it does**:
1. Detects current version from CMakeLists.txt
   - Uses grep and sed to extract version
   - Validates version format
2. Determines version bump type based on PR labels:
   - `major`: Breaking changes
   - `minor`: New features
   - `patch`: Bug fixes
3. Calculates new version using semver
   - Uses Python semver package
   - Handles version bumping
4. Updates CMakeLists.txt
   - Replaces old version with new
   - Creates commit with changes
5. Creates git tag
   - Tags with new version
   - Pushes tag to repository
6. Creates GitHub release
   - Uses PR description
   - Includes version change info

**Version Bump Rules**:
- `major` label → X.0.0 (breaking changes)
- `minor` label → 0.X.0 (new features)
- `patch` label → 0.0.X (bug fixes)

**Error Handling**:
- Validates version format
- Checks for empty versions
- Handles semver errors
- Provides detailed error messages

## Issue Handler Workflow (`issue-handler.yml`)

**Purpose**: Automates issue management and tracking.

**Triggers**:
- Issue opened
- Issue labeled
- Issue unlabeled
- Issue closed
- Issue reopened

**What it does**:
1. Detects issue type
2. Adds appropriate version labels
3. Creates feature branches
4. Updates issue status
5. Manages related PRs

**Label Management**:
- Automatically adds version labels
- Updates issue status based on labels
- Manages workflow labels

## How to Use

### For Developers
1. Create issues with appropriate labels
   - Use descriptive titles
   - Add relevant labels
   - Follow issue template
2. Work on feature branches
   - Branch from dev
   - Follow naming convention
   - Keep branches up to date
3. Create PRs with version labels
   - Add appropriate version label
   - Follow PR template
   - Link related issues
4. Merge PRs to trigger version updates
   - Ensure CI passes
   - Get required reviews
   - Merge to main

### For Maintainers
1. Review PRs
   - Check version labels
   - Verify changes
   - Ensure quality
2. Monitor automated releases
   - Check version updates
   - Verify release notes
   - Test release artifacts
3. Manage issue workflow
   - Review issue labels
   - Update issue status
   - Close resolved issues

### Version Control
- Use semantic versioning (MAJOR.MINOR.PATCH)
- Follow conventional commits
- Add appropriate labels
- Let automation handle versioning

## Best Practices

1. **PR Labels**:
   - Always add version labels
   - Use descriptive PR titles
   - Follow conventional commits
   - Link related issues

2. **Issue Management**:
   - Use appropriate issue types
   - Add relevant labels
   - Keep issues updated
   - Follow issue template

3. **Version Control**:
   - Don't manually update versions
   - Let automation handle bumps
   - Follow semantic versioning
   - Keep changelog updated

4. **Testing**:
   - Run tests locally
   - Check CI results
   - Verify version updates
   - Test release artifacts

5. **Documentation**:
   - Update README.md
   - Document changes
   - Keep workflows.md updated
   - Maintain changelog 