/*
 * Copyright (C) 2014 The CyanogenMod Project
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.lineageos.hardware;

import org.lineageos.internal.util.FileUtils;

public class DisplayColorCalibration {

    private static final String COLOR_RED_FILE = "/sys/module/msm_drm/parameters/red";
    private static final String COLOR_GREEN_FILE = "/sys/module/msm_drm/parameters/green";
    private static final String COLOR_BLUE_FILE = "/sys/module/msm_drm/parameters/blue";

    private static final int MIN = 1;
    private static final int MAX = 255;

    public static boolean isSupported() {
        return FileUtils.isFileReadable(COLOR_RED_FILE) &&
                FileUtils.isFileWritable(COLOR_RED_FILE) &&
                FileUtils.isFileReadable(COLOR_GREEN_FILE) &&
                FileUtils.isFileWritable(COLOR_GREEN_FILE) &&
                FileUtils.isFileReadable(COLOR_BLUE_FILE) &&
                FileUtils.isFileWritable(COLOR_BLUE_FILE);
    }

    public static int getMaxValue()  {
        return MAX;
    }

    public static int getMinValue()  {
        return MIN;
    }

    public static int getDefValue() {
        return getMaxValue();
    }

    public static String getCurColors()  {
        return String.format("%s %s %s",
                FileUtils.readOneLine(COLOR_RED_FILE),
                FileUtils.readOneLine(COLOR_GREEN_FILE),
                FileUtils.readOneLine(COLOR_BLUE_FILE));
    }

    public static boolean setColors(String colors) {
        String[] array = colors.split(" ");
        return FileUtils.writeLine(COLOR_RED_FILE, array[0]) &&
                FileUtils.writeLine(COLOR_GREEN_FILE, array[1]) &&
                FileUtils.writeLine(COLOR_BLUE_FILE, array[2]);
    }
}
