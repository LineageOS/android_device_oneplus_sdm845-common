#! /vendor/bin/sh

# Copyright (c) 2012-2013, 2016-2020, The Linux Foundation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of The Linux Foundation nor
#       the names of its contributors may be used to endorse or promote
#       products derived from this software without specific prior written
#       permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

target=`getprop ro.board.platform`

function configure_zram_parameters() {
    MemTotalStr=`cat /proc/meminfo | grep MemTotal`
    MemTotal=${MemTotalStr:16:8}

    low_ram=`getprop ro.config.low_ram`

    # Zram disk - 75% for Go devices.
    # For 512MB Go device, size = 384MB, set same for Non-Go.
    # For 1GB Go device, size = 768MB, set same for Non-Go.
    # For 2GB Go device, size = 1536MB, set same for Non-Go.
    # For >2GB Non-Go devices, size = 50% of RAM size. Limit the size to 4GB.
    # And enable lz4 zram compression for Go targets.

    let RamSizeGB="( $MemTotal / 1048576 ) + 1"
    diskSizeUnit=M
    if [ $RamSizeGB -le 2 ]; then
        let zRamSizeMB="( $RamSizeGB * 1024 ) * 3 / 4"
    else
        let zRamSizeMB="( $RamSizeGB * 1024 ) / 2"
    fi

    # use MB avoid 32 bit overflow
    if [ $zRamSizeMB -gt 4096 ]; then
        let zRamSizeMB=4096
    fi

    if [ "$low_ram" == "true" ]; then
        echo lz4 > /sys/block/zram0/comp_algorithm
    fi

    if [ -f /sys/block/zram0/disksize ]; then
        if [ -f /sys/block/zram0/use_dedup ]; then
            echo 1 > /sys/block/zram0/use_dedup
        fi
        echo "$zRamSizeMB""$diskSizeUnit" > /sys/block/zram0/disksize

        # ZRAM may use more memory than it saves if SLAB_STORE_USER
        # debug option is enabled.
        if [ -e /sys/kernel/slab/zs_handle ]; then
            echo 0 > /sys/kernel/slab/zs_handle/store_user
        fi
        if [ -e /sys/kernel/slab/zspage ]; then
            echo 0 > /sys/kernel/slab/zspage/store_user
        fi

        mkswap /dev/block/zram0
        swapon /dev/block/zram0 -p 32758
    fi
}

function configure_read_ahead_kb_values() {
    MemTotalStr=`cat /proc/meminfo | grep MemTotal`
    MemTotal=${MemTotalStr:16:8}

    dmpts=$(ls /sys/block/*/queue/read_ahead_kb | grep -e dm -e mmc)

    # Set 128 for <= 3GB &
    # set 512 for >= 4GB targets.
    if [ $MemTotal -le 3145728 ]; then
        echo 128 > /sys/block/mmcblk0/bdi/read_ahead_kb
        echo 128 > /sys/block/mmcblk0rpmb/bdi/read_ahead_kb
        for dm in $dmpts; do
            echo 128 > $dm
        done
    else
        echo 512 > /sys/block/mmcblk0/bdi/read_ahead_kb
        echo 512 > /sys/block/mmcblk0rpmb/bdi/read_ahead_kb
        for dm in $dmpts; do
            echo 512 > $dm
        done
    fi
}

function disable_core_ctl() {
    if [ -f /sys/devices/system/cpu/cpu0/core_ctl/enable ]; then
        echo 0 > /sys/devices/system/cpu/cpu0/core_ctl/enable
    else
        echo 1 > /sys/devices/system/cpu/cpu0/core_ctl/disable
    fi
}

function enable_swap() {
    MemTotalStr=`cat /proc/meminfo | grep MemTotal`
    MemTotal=${MemTotalStr:16:8}

    SWAP_ENABLE_THRESHOLD=1048576
    swap_enable=`getprop ro.vendor.qti.config.swap`

    # Enable swap initially only for 1 GB targets
    if [ "$MemTotal" -le "$SWAP_ENABLE_THRESHOLD" ] && [ "$swap_enable" == "true" ]; then
        # Static swiftness
        echo 1 > /proc/sys/vm/swap_ratio_enable
        echo 70 > /proc/sys/vm/swap_ratio

        # Swap disk - 200MB size
        if [ ! -f /data/vendor/swap/swapfile ]; then
            dd if=/dev/zero of=/data/vendor/swap/swapfile bs=1m count=200
        fi
        mkswap /data/vendor/swap/swapfile
        swapon /data/vendor/swap/swapfile -p 32758
    fi
}

function configure_memory_parameters() {
    # Set Memory parameters.
    #
    # Set per_process_reclaim tuning parameters
    # All targets will use vmpressure range 50-70,
    # All targets will use 512 pages swap size.
    #
    # Set Low memory killer minfree parameters
    # 32 bit Non-Go, all memory configurations will use 15K series
    # 32 bit Go, all memory configurations will use uLMK + Memcg
    # 64 bit will use Google default LMK series.
    #
    # Set ALMK parameters (usually above the highest minfree values)
    # vmpressure_file_min threshold is always set slightly higher
    # than LMK minfree's last bin value for all targets. It is calculated as
    # vmpressure_file_min = (last bin - second last bin ) + last bin
    #
    # Set allocstall_threshold to 0 for all targets.
    #

ProductName=`getprop ro.product.name`
low_ram=`getprop ro.config.low_ram`

if [ "$ProductName" == "msmnile" ] || [ "$ProductName" == "kona" ] || [ "$ProductName" == "sdmshrike_au" ]; then
      # Enable ZRAM
      configure_zram_parameters
      configure_read_ahead_kb_values
      echo 0 > /proc/sys/vm/page-cluster
      echo 100 > /proc/sys/vm/swappiness
else
    arch_type=`uname -m`

    # Set parameters for 32-bit Go targets.
    if [ "$low_ram" == "true" ]; then
        # Disable KLMK, ALMK, PPR & Core Control for Go devices
        echo 0 > /sys/module/lowmemorykiller/parameters/enable_lmk
        echo 0 > /sys/module/lowmemorykiller/parameters/enable_adaptive_lmk
        echo 0 > /sys/module/process_reclaim/parameters/enable_process_reclaim
        disable_core_ctl
        # Enable oom_reaper for Go devices
        if [ -f /proc/sys/vm/reap_mem_on_sigkill ]; then
            echo 1 > /proc/sys/vm/reap_mem_on_sigkill
        fi
    else

        # Read adj series and set adj threshold for PPR and ALMK.
        # This is required since adj values change from framework to framework.
        adj_series=`cat /sys/module/lowmemorykiller/parameters/adj`
        adj_1="${adj_series#*,}"
        set_almk_ppr_adj="${adj_1%%,*}"

        # PPR and ALMK should not act on HOME adj and below.
        # Normalized ADJ for HOME is 6. Hence multiply by 6
        # ADJ score represented as INT in LMK params, actual score can be in decimal
        # Hence add 6 considering a worst case of 0.9 conversion to INT (0.9*6).
        # For uLMK + Memcg, this will be set as 6 since adj is zero.
        set_almk_ppr_adj=$(((set_almk_ppr_adj * 6) + 6))
        echo $set_almk_ppr_adj > /sys/module/lowmemorykiller/parameters/adj_max_shift

        # Calculate vmpressure_file_min as below & set for 64 bit:
        # vmpressure_file_min = last_lmk_bin + (last_lmk_bin - last_but_one_lmk_bin)
        if [ "$arch_type" == "aarch64" ]; then
            minfree_series=`cat /sys/module/lowmemorykiller/parameters/minfree`
            minfree_1="${minfree_series#*,}" ; rem_minfree_1="${minfree_1%%,*}"
            minfree_2="${minfree_1#*,}" ; rem_minfree_2="${minfree_2%%,*}"
            minfree_3="${minfree_2#*,}" ; rem_minfree_3="${minfree_3%%,*}"
            minfree_4="${minfree_3#*,}" ; rem_minfree_4="${minfree_4%%,*}"
            minfree_5="${minfree_4#*,}"

            vmpres_file_min=$((minfree_5 + (minfree_5 - rem_minfree_4)))
            echo $vmpres_file_min > /sys/module/lowmemorykiller/parameters/vmpressure_file_min
        else
            # Set LMK series, vmpressure_file_min for 32 bit non-go targets.
            # Disable Core Control, enable KLMK for non-go 8909.
            if [ "$ProductName" == "msm8909" ]; then
                disable_core_ctl
                echo 1 > /sys/module/lowmemorykiller/parameters/enable_lmk
            fi
        echo "15360,19200,23040,26880,34415,43737" > /sys/module/lowmemorykiller/parameters/minfree
        echo 53059 > /sys/module/lowmemorykiller/parameters/vmpressure_file_min
        fi

        # Enable adaptive LMK for all targets &
        # use Google default LMK series for all 64-bit targets >=2GB.
        echo 1 > /sys/module/lowmemorykiller/parameters/enable_adaptive_lmk

        # Enable oom_reaper
        if [ -f /sys/module/lowmemorykiller/parameters/oom_reaper ]; then
            echo 1 > /sys/module/lowmemorykiller/parameters/oom_reaper
        fi

        if [[ "$ProductName" != "bengal"* ]]; then
            #bengal has appcompaction enabled. So not needed
            # Set PPR parameters for other targets
            if [ -f /sys/devices/soc0/soc_id ]; then
                soc_id=`cat /sys/devices/soc0/soc_id`
            else
                soc_id=`cat /sys/devices/system/soc/soc0/id`
            fi

            case "$soc_id" in
              # Do not set PPR parameters for premium targets
              # sdm845 - 321, 341
              # msm8998 - 292, 319
              # msm8996 - 246, 291, 305, 312
              "321" | "341" | "292" | "319" | "246" | "291" | "305" | "312")
                ;;
              *)
                #Set PPR parameters for all other targets.
                echo $set_almk_ppr_adj > /sys/module/process_reclaim/parameters/min_score_adj
                echo 1 > /sys/module/process_reclaim/parameters/enable_process_reclaim
                echo 50 > /sys/module/process_reclaim/parameters/pressure_min
                echo 70 > /sys/module/process_reclaim/parameters/pressure_max
                echo 30 > /sys/module/process_reclaim/parameters/swap_opt_eff
                echo 512 > /sys/module/process_reclaim/parameters/per_swap_size
                ;;
            esac
        fi
    fi

    if [[ "$ProductName" == "bengal"* ]]; then
        #Set PPR nomap parameters for bengal targets
        echo 1 > /sys/module/process_reclaim/parameters/enable_process_reclaim
        echo 50 > /sys/module/process_reclaim/parameters/pressure_min
        echo 70 > /sys/module/process_reclaim/parameters/pressure_max
        echo 30 > /sys/module/process_reclaim/parameters/swap_opt_eff
        echo 0 > /sys/module/process_reclaim/parameters/per_swap_size
        echo 7680 > /sys/module/process_reclaim/parameters/tsk_nomap_swap_sz
    fi

    # Set allocstall_threshold to 0 for all targets.
    # Set swappiness to 100 for all targets
    echo 0 > /sys/module/vmpressure/parameters/allocstall_threshold
    echo 100 > /proc/sys/vm/swappiness

    # Disable wsf for all targets beacause we are using efk.
    # wsf Range : 1..1000 So set to bare minimum value 1.
    echo 1 > /proc/sys/vm/watermark_scale_factor

    configure_zram_parameters

    configure_read_ahead_kb_values

    enable_swap
fi
}

function start_hbtp()
{
        # Start the Host based Touch processing but not in the power off mode.
        bootmode=`getprop ro.bootmode`
        if [ "charger" != $bootmode ]; then
                start vendor.hbtp
        fi
}

case "$target" in
    "sdm845")

        # Set the default IRQ affinity to the silver cluster. When a
        # CPU is isolated/hotplugged, the IRQ affinity is adjusted
        # to one of the CPU from the default IRQ affinity mask.
        echo f > /proc/irq/default_smp_affinity

        if [ -f /sys/devices/soc0/soc_id ]; then
                soc_id=`cat /sys/devices/soc0/soc_id`
        else
                soc_id=`cat /sys/devices/system/soc/soc0/id`
        fi

        if [ -f /sys/devices/soc0/hw_platform ]; then
                hw_platform=`cat /sys/devices/soc0/hw_platform`
        fi

        if [ -f /sys/devices/soc0/platform_subtype_id ]; then
                platform_subtype_id=`cat /sys/devices/soc0/platform_subtype_id`
        fi

        case "$soc_id" in
                "321" | "341")
                # Start Host based Touch processing
                case "$hw_platform" in
                    "QRD" )
                            case "$platform_subtype_id" in
                                   "32") #QVR845 do nothing
                                     ;;
                                   *)
                                         start_hbtp
                                     ;;
                            esac
                     ;;
                    *)
                          start_hbtp
                     ;;
                esac
         ;;
        esac
    # Core control parameters
    echo 2 > /sys/devices/system/cpu/cpu4/core_ctl/min_cpus
    echo 60 > /sys/devices/system/cpu/cpu4/core_ctl/busy_up_thres
    echo 30 > /sys/devices/system/cpu/cpu4/core_ctl/busy_down_thres
    echo 100 > /sys/devices/system/cpu/cpu4/core_ctl/offline_delay_ms
    echo 1 > /sys/devices/system/cpu/cpu4/core_ctl/is_big_cluster
    echo 4 > /sys/devices/system/cpu/cpu4/core_ctl/task_thres

    # Setting b.L scheduler parameters
    echo 95 > /proc/sys/kernel/sched_upmigrate
    echo 85 > /proc/sys/kernel/sched_downmigrate
    echo 100 > /proc/sys/kernel/sched_group_upmigrate
    echo 95 > /proc/sys/kernel/sched_group_downmigrate
    echo 1 > /proc/sys/kernel/sched_walt_rotate_big_tasks

    # configure governor settings for little cluster
    echo "schedutil" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
    echo 0 > /sys/devices/system/cpu/cpu0/cpufreq/schedutil/rate_limit_us
    echo 1228800 > /sys/devices/system/cpu/cpu0/cpufreq/schedutil/hispeed_freq
    echo 1 > /sys/devices/system/cpu/cpu0/cpufreq/schedutil/pl
        echo 576000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq

    # configure governor settings for big cluster
    echo "schedutil" > /sys/devices/system/cpu/cpu4/cpufreq/scaling_governor
    echo 0 > /sys/devices/system/cpu/cpu4/cpufreq/schedutil/rate_limit_us
    echo 1536000 > /sys/devices/system/cpu/cpu4/cpufreq/schedutil/hispeed_freq
    echo 1 > /sys/devices/system/cpu/cpu4/cpufreq/schedutil/pl
    echo "0:1324800" > /sys/module/cpu_boost/parameters/input_boost_freq
    echo 120 > /sys/module/cpu_boost/parameters/input_boost_ms
    # Limit the min frequency to 825MHz
    echo 825000 > /sys/devices/system/cpu/cpu4/cpufreq/scaling_min_freq

        # Enable oom_reaper
        echo 1 > /sys/module/lowmemorykiller/parameters/oom_reaper

        # Enable bus-dcvs
        for cpubw in /sys/class/devfreq/*qcom,cpubw*
        do
            echo "bw_hwmon" > $cpubw/governor
            echo 50 > $cpubw/polling_interval
            echo "2288 4577 6500 8132 9155 10681" > $cpubw/bw_hwmon/mbps_zones
            echo 4 > $cpubw/bw_hwmon/sample_ms
            echo 50 > $cpubw/bw_hwmon/io_percent
            echo 20 > $cpubw/bw_hwmon/hist_memory
            echo 10 > $cpubw/bw_hwmon/hyst_length
            echo 0 > $cpubw/bw_hwmon/guard_band_mbps
            echo 250 > $cpubw/bw_hwmon/up_scale
            echo 1600 > $cpubw/bw_hwmon/idle_mbps
        done

        for llccbw in /sys/class/devfreq/*qcom,llccbw*
        do
            echo "bw_hwmon" > $llccbw/governor
            echo 50 > $llccbw/polling_interval
            echo "1720 2929 3879 5931 6881" > $llccbw/bw_hwmon/mbps_zones
            echo 4 > $llccbw/bw_hwmon/sample_ms
            echo 80 > $llccbw/bw_hwmon/io_percent
            echo 20 > $llccbw/bw_hwmon/hist_memory
            echo 10 > $llccbw/bw_hwmon/hyst_length
            echo 0 > $llccbw/bw_hwmon/guard_band_mbps
            echo 250 > $llccbw/bw_hwmon/up_scale
            echo 1600 > $llccbw/bw_hwmon/idle_mbps
        done

    #Enable mem_latency governor for DDR scaling
        for memlat in /sys/class/devfreq/*qcom,memlat-cpu*
        do
    echo "mem_latency" > $memlat/governor
            echo 10 > $memlat/polling_interval
            echo 400 > $memlat/mem_latency/ratio_ceil
        done

    #Enable mem_latency governor for L3 scaling
        for memlat in /sys/class/devfreq/*qcom,l3-cpu*
        do
            echo "mem_latency" > $memlat/governor
            echo 10 > $memlat/polling_interval
            echo 400 > $memlat/mem_latency/ratio_ceil
        done

        #Enable userspace governor for L3 cdsp nodes
        for l3cdsp in /sys/class/devfreq/*qcom,l3-cdsp*
        do
            echo "userspace" > $l3cdsp/governor
            chown -h system $l3cdsp/userspace/set_freq
        done

    #Gold L3 ratio ceil
        echo 4000 > /sys/class/devfreq/soc:qcom,l3-cpu4/mem_latency/ratio_ceil

    echo "compute" > /sys/class/devfreq/soc:qcom,mincpubw/governor
    echo 10 > /sys/class/devfreq/soc:qcom,mincpubw/polling_interval

    # cpuset parameters
        echo 0-3 > /dev/cpuset/background/cpus
        echo 0-3 > /dev/cpuset/system-background/cpus

    # Turn off scheduler boost at the end
        echo 0 > /proc/sys/kernel/sched_boost
    # Disable CPU Retention
        echo N > /sys/module/lpm_levels/L3/cpu0/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu1/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu2/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu3/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu4/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu5/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu6/ret/idle_enabled
        echo N > /sys/module/lpm_levels/L3/cpu7/ret/idle_enabled
    echo N > /sys/module/lpm_levels/L3/l3-dyn-ret/idle_enabled
        # Turn on sleep modes.
        echo 0 > /sys/module/lpm_levels/parameters/sleep_disabled
    echo 100 > /proc/sys/vm/swappiness
    echo 120 > /proc/sys/vm/watermark_scale_factor
    ;;
esac
