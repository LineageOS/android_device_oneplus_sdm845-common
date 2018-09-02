/*
 * Copyright (c) 2016 The CyanogenMod Project
 * Copyright (c) 2018 The LineageOS Project
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

package org.lineageos.pocketmode;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.text.TextUtils;
import android.util.Log;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class PocketSensor implements SensorEventListener {
    private static final boolean DEBUG = false;
    private static final String TAG = "PocketSensor";

    private static final String GOODIX_FILE =
            "/sys/devices/platform/soc/soc:goodix_fp/proximity_state";

    private ExecutorService mExecutorService;
    private SensorManager mSensorManager;
    private Sensor mSensor;
    private Context mContext;

    public PocketSensor(Context context) {
        mContext = context;
        mSensorManager = mContext.getSystemService(SensorManager.class);
        mExecutorService = Executors.newSingleThreadExecutor();

        for (Sensor sensor : mSensorManager.getSensorList(Sensor.TYPE_ALL)) {
            if (TextUtils.equals(sensor.getStringType(), "oneplus.sensor.pocket")) {
                mSensor = sensor;
                break;
            }
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        setFPProximityState(event.values[0] == 1.0);
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
        /* Empty */
    }

    private void setFPProximityState(boolean isNear) {
        if (!FileUtils.writeLine(GOODIX_FILE, isNear ? "1" : "0")) {
            Log.e(TAG, "Proximity state file " + GOODIX_FILE + " is not writable!");
        }
    }

    void enable() {
        if (DEBUG) Log.d(TAG, "Enabling");
        mExecutorService.submit(() -> {
            mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_NORMAL);
        });
    }

    void disable() {
        if (DEBUG) Log.d(TAG, "Disabling");
        mExecutorService.submit(() -> {
            mSensorManager.unregisterListener(this, mSensor);
            // Ensure FP is left enabled
            setFPProximityState(/* isNear */ false);
        });
    }
}
