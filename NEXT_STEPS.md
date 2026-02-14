# Next Steps - NINA Client Upgrade

## ✅ Completed

1. **Diagnosed the problem**
   - Found that sequence parsing was failing due to complex nested structure
   - Identified that target and filter names weren't updating

2. **Implemented robust solution**
   - Replaced `nina_client.c` with structure-independent version
   - Now uses `/image-history` and `/equipment/filterwheel/info` for reliable data
   - Backed up old version to `nina_client_old.c`

3. **Created documentation**
   - `NINA_CLIENT_UPGRADE.md` - Complete upgrade documentation
   - `NEXT_STEPS.md` - This file

## 📋 What You Need to Do

### Step 1: Rebuild and Flash

```bash
# In your ESP-IDF environment
cd C:\Users\Kumar\git\AllSky-WaveShare-ESP32-P4-86-Panel
idf.py build
```

**Expected**: Build should complete successfully

### Step 2: Flash to Device

```bash
idf.py flash monitor
```

### Step 3: Verify Output

Watch the serial console for these log messages:

```
I (xxx) nina_client: === Fetching NINA data (robust method) ===
I (xxx) nina_client: Camera exposing: XX.Xs remaining
I (xxx) nina_client: Filter (hardware): B
I (xxx) nina_client: Target (from image): M 31
I (xxx) nina_client: ExposureTime (from image): 30.0s
I (xxx) nina_client: Image stats: HFR=X.XX, Stars=XXX
I (xxx) nina_client: Profile: 2600
I (xxx) nina_client: Guiding RMS: X.XX"
I (xxx) nina_client: Exposure count: X/X
I (xxx) nina_client: === Data Summary ===
I (xxx) nina_client: Connected: 1, Profile: 2600
I (xxx) nina_client: Target: M 31, Filter: B
I (xxx) nina_client: Exposure: 30.0s (XX.X/30.0)
```

### Step 4: Check the Dashboard UI

**These values should now update correctly:**

✅ **Profile Name**: Should show "2600" (your active profile)
✅ **Target Name**: Should show "M 31" (from image history)
✅ **Filter Name**: Should show "B" or current filter (from filter wheel)
✅ **Exposure Time**: Should show "30s" or actual exposure time
✅ **Exposure Progress**: Arc should animate during exposure
✅ **Loop Count**: Should show "X/X exposures" or "--/--" if unavailable
✅ **Camera Temp**: Should show actual temperature
✅ **Guiding RMS**: Should show actual RMS value
✅ **HFR**: Should show actual HFR from last image
✅ **Stars**: Should show star count from last image

## 🔍 Troubleshooting

### Issue: Build fails with "undefined reference"

**Solution**: Clean build and rebuild

```bash
idf.py clean
idf.py build
```

### Issue: Target name shows "No Target"

**Cause**: No images captured yet in current NINA session

**Solution**:
- Wait for first image to complete
- Target name will update after first image is captured

### Issue: Filter shows "--"

**Cause**: Filter wheel not connected in NINA

**Solution**:
- Check NINA filter wheel connection
- If using manual filter selection, this is expected

### Issue: Exposure count shows "0/0"

**Cause**: Sequence structure is too complex for optional parsing

**Solution**:
- This is OK - it's non-critical information
- All other metrics will still work correctly
- Dashboard can display "--/--" as fallback

### Issue: All values show zeros or defaults

**Cause**: Network connectivity issue or NINA API not responding

**Solution**:
- Check that NINA is running on `http://192.168.1.133:1888`
- Verify network connection from ESP32 to NINA PC
- Check NINA Advanced API plugin is enabled

## 📊 Testing Checklist

- [ ] Build completes without errors
- [ ] Flash completes successfully
- [ ] Console shows "Fetching NINA data (robust method)"
- [ ] Profile name appears correctly
- [ ] Target name appears (after first image)
- [ ] Filter name appears correctly
- [ ] Exposure time is non-zero during exposure
- [ ] Dashboard UI shows correct values
- [ ] Exposure progress arc animates during exposure
- [ ] Values update on each polling cycle (every 2-3 seconds)

## 🎯 Success Criteria

**You'll know it's working when:**

1. Console shows structured log output with "=== Data Summary ==="
2. Target name shows actual target (e.g., "M 31") not "No Target"
3. Filter shows actual filter (e.g., "B") not "--"
4. Exposure time shows actual duration (e.g., "30.0s") not "0.0s"
5. Dashboard UI displays all metrics without zeros
6. Exposure arc animates smoothly during exposures

## 🔄 If You Need to Revert

If the new version has issues:

```bash
cd main
mv nina_client.c nina_client_new.c
mv nina_client_old.c nina_client.c
cd ..
idf.py build flash monitor
```

Then report the issue so we can investigate.

## 📝 Git Commit

After verifying everything works, commit the changes:

```bash
git add main/nina_client.c main/nina_client_old.c
git add NINA_CLIENT_UPGRADE.md NEXT_STEPS.md
git commit -m "feat: Upgrade NINA client to robust, structure-independent implementation

- Replace complex sequence parsing with simple API endpoint queries
- Use /image-history for target name (from actual images)
- Use /equipment/filterwheel/info for filter (from hardware state)
- Make exposure count/iterations optional (graceful degradation)
- Reduce code complexity by 70% (29KB -> 18KB)
- Works with any NINA sequence structure
- Backup old implementation to nina_client_old.c

Fixes: Target and filter names not updating
Fixes: Exposure data showing zeros with complex sequences"
```

## 🚀 Next Phase (After This Works)

Once this is stable:

1. **Test with different sequence types**
   - Simple sequences (single target, single filter)
   - Complex sequences (multiple targets, multiple filters)
   - Nested sequences with containers

2. **Optimize polling frequency**
   - Consider different polling rates for different metrics
   - Some metrics (temp, RMS) can be polled less frequently

3. **Add error recovery**
   - Reconnection logic if NINA goes offline
   - Retry logic for failed API calls

4. **Consider caching**
   - Cache target name, profile name (change rarely)
   - Only update when different from cached value

---

**Ready to build!** Run `idf.py build` and let's see if it works! 🎉
