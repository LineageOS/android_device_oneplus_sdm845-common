/*
 * Copyright (C) 2015 The CyanogenMod Project
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

import android.util.Log;

import lineageos.hardware.DisplayMode;
import org.lineageos.internal.util.FileUtils;

import java.util.HashMap;

/*
 * Display Modes API
 *
 * A device may implement a list of preset display modes for different
 * viewing intents, such as movies, photos, or extra vibrance. These
 * modes may have multiple components such as gamma correction, white
 * point adjustment, etc, but are activated by a single control point.
 *
 * This API provides support for enumerating and selecting the
 * modes supported by the hardware.
 */

public class DisplayModeControl {
    private static final String MODE_PATH =
            "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/main_display/display_mode";
    private static final String DEFAULT_PATH = "/data/system/default_display_mode";

    private static final HashMap<String, DisplayMode> MODE_MAP = new HashMap<>();
    static {
        MODE_MAP.put("default", new DisplayMode(0, "Standard"));
        MODE_MAP.put("srgb", new DisplayMode(1, "sRGB"));
        MODE_MAP.put("dci-p3", new DisplayMode(2, "DCI P3"));
        MODE_MAP.put("adaption", new DisplayMode(3, "Adaptive"));

        String storedDefaultMode = FileUtils.readOneLine(DEFAULT_PATH);
        if (storedDefaultMode != null) {
            FileUtils.writeLine(MODE_PATH, storedDefaultMode);
        }
    }

    /*
     * All HAF classes should export this boolean.
     * Real implementations must, of course, return true
     */
    public static boolean isSupported() {
        return FileUtils.isFileWritable(MODE_PATH);
    }

    /*
     * Get the list of available modes. A mode has an integer
     * identifier and a string name.
     *
     * It is the responsibility of the upper layers to
     * map the name to a human-readable format or perform translation.
     */
    public static DisplayMode[] getAvailableModes() {
        return MODE_MAP.values().toArray(new DisplayMode[MODE_MAP.size()]);
    }

    /*
     * Get the name of the currently selected mode. This can return
     * null if no mode is selected.
     */
    public static DisplayMode getCurrentMode() {
        return MODE_MAP.get(FileUtils.readOneLine(MODE_PATH));
    }

    /*
     * Selects a mode from the list of available modes by it's
     * string identifier. Returns true on success, false for
     * failure. It is up to the implementation to determine
     * if this mode is valid.
     */
    public static boolean setMode(DisplayMode mode, boolean makeDefault) {
        for (HashMap.Entry<String, DisplayMode> entry : MODE_MAP.entrySet()) {
            if (entry.getValue().id == mode.id) {
                if (FileUtils.writeLine(MODE_PATH, entry.getKey()) && makeDefault) {
                    FileUtils.writeLine(DEFAULT_PATH, entry.getKey());
                    return true;
                }
            }
        }
        return false;
    }

    /*
     * Gets the preferred default mode for this device by it's
     * string identifier. Can return null if there is no default.
     */
    public static DisplayMode getDefaultMode() {
        String storedDefaultMode = FileUtils.readOneLine(DEFAULT_PATH);
        return MODE_MAP.get(storedDefaultMode != null ? storedDefaultMode : "default");
    }
}
