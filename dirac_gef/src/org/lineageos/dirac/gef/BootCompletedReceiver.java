/*
 * Copyright (c) 2021 The LineageOS Project
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

package org.lineageos.dirac.gef;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.media.audiofx.AudioEffect;
import android.util.Log;

import java.util.UUID;

public class BootCompletedReceiver extends BroadcastReceiver {
    private static final String TAG = "OnePlusDiracGef";

    private static final UUID EFFECT_TYPE_DIRAC_GEF =
            UUID.fromString("3799d6d1-22c5-43c3-b3ec-d664cf8d2f0d");

    private static AudioEffect sAudioEffect;
                
    @Override
    public void onReceive(final Context context, Intent intent) {
        Log.d(TAG, "Starting");
        sAudioEffect = new AudioEffect(AudioEffect.EFFECT_TYPE_NULL, EFFECT_TYPE_DIRAC_GEF, 0, 0);
        sAudioEffect.setEnabled(true);
    }
}
