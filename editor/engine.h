/* Copyright (C) 2012-2014 Carlos Pais
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ENGINE_H
#define ENGINE_H

#include <QString>

#if defined(Q_OS_MAC)
    #define ENGINE_DEFAULT_PATH "../Resources/engine"
#else
    #define ENGINE_DEFAULT_PATH  "engine"
#endif

class Engine {

public:

    static bool isValidPath(const QString&);
    static bool isValid();
    static QString path();
    static bool loadPath();
    static QString defaultPath();
    static bool loadDefaultPath();
    static void setPath(const QString&);
    static bool pathChanged();
    static QString browserPath();
    static void setBrowserPath(const QString&);
    static void setUseBuiltinBrowser(bool);
    static bool useBuiltinBrowser();
};

#endif // ENGINE_H
