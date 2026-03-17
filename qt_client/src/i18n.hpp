#pragma once

#include <QString>

enum class AppLang { ZhCN, EnUS };

inline QString tr2(AppLang lang, const QString& zh, const QString& en) {
    return (lang == AppLang::ZhCN) ? zh : en;
}
