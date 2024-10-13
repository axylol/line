#pragma once

bool isPathWhitelisted(const char* path);
void fixPathIfNeeded(char* out, const char* path);

void InitPathWhitelist();