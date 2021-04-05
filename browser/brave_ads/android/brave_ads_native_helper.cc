/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_ads/android/brave_ads_native_helper.h"

#include <memory>
#include <string>

#include "base/android/jni_string.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/components/brave_ads/browser/ads_service.h"
#include "brave/components/brave_ads/browser/ads_service_factory.h"
#include "brave/components/brave_rewards/browser/rewards_service.h"
#include "brave/components/brave_rewards/browser/rewards_service_factory.h"
#include "chrome/browser/profiles/profile_android.h"

namespace brave_ads {

// static
jboolean JNI_BraveAdsNativeHelper_IsBraveAdsEnabled(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  return ads_service->IsEnabled();
}

// static
jboolean JNI_BraveAdsNativeHelper_IsLocaleValid(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  return ads_service->IsSupportedLocale();
}

// static
jboolean JNI_BraveAdsNativeHelper_IsSupportedLocale(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  return ads_service->IsSupportedLocale();
}

// static
jboolean JNI_BraveAdsNativeHelper_IsNewlySupportedLocale(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  return ads_service->IsNewlySupportedLocale();
}

// static
void JNI_BraveAdsNativeHelper_SetAdsEnabled(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  RewardsService* rewards_service =
      brave_rewards::RewardsServiceFactory::GetForProfile(profile);
  DCHECK(rewards_service);

  rewards_service->SetAdsEnabled(true);
}

// static
void JNI_BraveAdsNativeHelper_OnShowAdNotification(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android,
    const base::android::JavaParamRef<jstring>& j_notification_id,
    jboolean j_by_user) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  const std::string notification_id =
      base::android::ConvertJavaStringToUTF8(env, j_notification_id);
  ads_service->OnShowAdNotification(notification_id);
}

// static
void JNI_BraveAdsNativeHelper_OnCloseAdNotification(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android,
    const base::android::JavaParamRef<jstring>& j_notification_id,
    jboolean j_by_user) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  const std::string notification_id =
      base::android::ConvertJavaStringToUTF8(env, j_notification_id);
  ads_service->OnCloseAdNotification(notification_id, j_by_user);
}

// static
void JNI_BraveAdsNativeHelper_OnClickAdNotification(
    JNIEnv* env,
    const base::android::JavaParamRef<jobject>& j_profile_android,
    const base::android::JavaParamRef<jstring>& j_notification_id) {
  Profile* profile = ProfileAndroid::FromProfileAndroid(j_profile_android);
  AdsService* ads_service = AdsServiceFactory::GetForProfile(profile);
  DCHECK(ads_service);

  const std::string notification_id =
      base::android::ConvertJavaStringToUTF8(env, j_notification_id);
  ads_service->OnClickAdNotification(notification_id);
}

}  // namespace brave_ads
