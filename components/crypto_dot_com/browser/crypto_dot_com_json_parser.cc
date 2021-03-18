/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <string>

#include "brave/components/crypto_dot_com/browser/crypto_dot_com_json_parser.h"

#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"

namespace {

double CalculateAssetVolume(double v, double h, double l) {
  // Volume is v * ((h + l) / 2)
  return  v * ((h + l) / 2.0);
}

}  // namespace

bool CryptoDotComJSONParser::GetTickerInfoFromJSON(
    const std::string& json,
    CryptoDotComTickerInfo* info) {
  if (!info) {
    return false;
  }

  base::JSONReader::ValueWithError value_with_error =
      base::JSONReader::ReadAndReturnValueWithError(
          json, base::JSONParserOptions::JSON_PARSE_RFC);
  base::Optional<base::Value>& records_v = value_with_error.value;

  if (!records_v) {
    LOG(ERROR) << "Invalid response, could not parse JSON, JSON is: " << json;
    info->insert({"volume", 0});
    info->insert({"price", 0});
    return false;
  }

  const base::Value* data = records_v->FindPath("response.result.data");
  if (!data) {
    // Empty values on failed response
    info->insert({"volume", 0});
    info->insert({"price", 0});
    return false;
  }

  const base::Value* v = data->FindKey("v");
  const base::Value* h = data->FindKey("h");
  const base::Value* l = data->FindKey("l");
  const base::Value* price = data->FindKey("a");

  // Number could be double or int.
  if (!(v && (v->is_double() || v->is_int())) ||
      !(h && (h->is_double() || h->is_int())) ||
      !(l && (l->is_double() || l->is_int())) ||
      !(price && (price->is_double() || price->is_int()))) {
    info->insert({"volume", 0});
    info->insert({"price", 0});
    return false;
  }

  const double volume = CalculateAssetVolume(
      v->GetDouble(), h->GetDouble(), l->GetDouble());

  info->insert({"volume", volume});
  info->insert({"price", price->GetDouble()});

  return true;
}

bool CryptoDotComJSONParser::GetChartDataFromJSON(
    const std::string& json,
    CryptoDotComChartData* data) {
  if (!data) {
    return false;
  }

  base::JSONReader::ValueWithError value_with_error =
      base::JSONReader::ReadAndReturnValueWithError(
          json, base::JSONParserOptions::JSON_PARSE_RFC);
  base::Optional<base::Value>& records_v = value_with_error.value;

  const std::map<std::string, double> empty_data_point = {
    {"t", 0}, {"o", 0}, {"h", 0}, {"l", 0}, {"c", 0}, {"v", 0}
  };

  if (!records_v) {
    LOG(ERROR) << "Invalid response, could not parse JSON, JSON is: " << json;
    data->push_back(empty_data_point);
    return false;
  }

  const base::Value* data_arr = records_v->FindPath("response.result.data");
  if (!data_arr || !data_arr->is_list()) {
    data->push_back(empty_data_point);
    return false;
  }

  for (const base::Value &point : data_arr->GetList()) {
    std::map<std::string, double> data_point;
    const base::Value* t = point.FindKey("t");
    const base::Value* o = point.FindKey("o");
    const base::Value* h = point.FindKey("h");
    const base::Value* l = point.FindKey("l");
    const base::Value* c = point.FindKey("c");
    const base::Value* v = point.FindKey("v");

    if (!(t && t->is_double()) ||
        !(o && o->is_double()) ||
        !(h && h->is_double()) ||
        !(l && l->is_double()) ||
        !(c && c->is_double()) ||
        !(v && v->is_double())) {
      data->clear();
      data->push_back(empty_data_point);
      return false;
    }

    data_point.insert({"t", t->GetDouble()});
    data_point.insert({"o", o->GetDouble()});
    data_point.insert({"h", h->GetDouble()});
    data_point.insert({"l", l->GetDouble()});
    data_point.insert({"c", c->GetDouble()});
    data_point.insert({"v", v->GetDouble()});

    data->push_back(data_point);
  }

  return true;
}

bool CryptoDotComJSONParser::GetPairsFromJSON(
    const std::string& json,
    CryptoDotComSupportedPairs* pairs) {
  if (!pairs) {
    return false;
  }

  base::JSONReader::ValueWithError value_with_error =
      base::JSONReader::ReadAndReturnValueWithError(
          json, base::JSONParserOptions::JSON_PARSE_RFC);
  base::Optional<base::Value>& records_v = value_with_error.value;

  const std::map<std::string, std::string> empty_pair = {
    {"pair", ""}, {"quote", ""}, {"base", ""}, {"price", ""}, {"quantity", ""}
  };

  if (!records_v) {
    LOG(ERROR) << "Invalid response, could not parse JSON, JSON is: " << json;
    pairs->push_back(empty_pair);
    return false;
  }

  const base::Value* instruments =
      records_v->FindPath("response.result.instruments");
  if (!instruments || !instruments->is_list()) {
    pairs->push_back(empty_pair);
    return false;
  }

  for (const base::Value &instrument : instruments->GetList()) {
    std::map<std::string, std::string> instrument_data;
    const base::Value* pair = instrument.FindKey("instrument_name");
    const base::Value* quote = instrument.FindKey("quote_currency");
    const base::Value* base = instrument.FindKey("base_currency");
    const base::Value* price = instrument.FindKey("price_decimals");
    const base::Value* quantity = instrument.FindKey("quantity_decimals");

    if (!(pair && pair->is_string()) ||
        !(quote && quote->is_string()) ||
        !(base && base->is_string()) ||
        !(price && price->is_int()) ||
        !(quantity && quantity->is_int())) {
      pairs->clear();
      pairs->push_back(empty_pair);
      return false;
    }

    instrument_data.insert({"pair", pair->GetString()});
    instrument_data.insert({"quote", quote->GetString()});
    instrument_data.insert({"base", base->GetString()});
    instrument_data.insert({"price", std::to_string(price->GetInt())});
    instrument_data.insert({"quantity", std::to_string(quantity->GetInt())});

    pairs->push_back(instrument_data);
  }

  return true;
}

bool CryptoDotComJSONParser::GetRankingsFromJSON(
    const std::string& json,
    CryptoDotComAssetRankings* rankings) {
  if (!rankings) {
    return false;
  }

  base::JSONReader::ValueWithError value_with_error =
      base::JSONReader::ReadAndReturnValueWithError(
          json, base::JSONParserOptions::JSON_PARSE_RFC);
  base::Optional<base::Value>& records_v = value_with_error.value;

  std::vector<std::map<std::string, std::string>> gainers;
  std::vector<std::map<std::string, std::string>> losers;

  if (!records_v) {
    LOG(ERROR) << "Invalid response, could not parse JSON, JSON is: " << json;
    rankings->insert({"gainers", gainers});
    rankings->insert({"losers", losers});
    return false;
  }

  const base::Value* result = records_v->FindPath("response.result");
  if (!result) {
    rankings->insert({"gainers", gainers});
    rankings->insert({"losers", losers});
    return false;
  }

  // Both gainers and losers are part of the "gainers" list
  const base::Value* rankings_list = result->FindKey("gainers");
  if (!rankings_list || !rankings_list->is_list()) {
    // Gainers and losers should return empty on a bad response
    rankings->insert({"gainers", gainers});
    rankings->insert({"losers", losers});
    return false;
  }

  for (const base::Value &ranking : rankings_list->GetList()) {
    std::map<std::string, std::string> ranking_data;
    const base::Value* pair = ranking.FindKey("instrument_name");
    const base::Value* change = ranking.FindKey("percent_change");
    const base::Value* last = ranking.FindKey("last_price");

    if (!pair || !pair->is_string() ||
        !change || !change->is_string() ||
        !last || !last->is_string()) {
      continue;
    }

    double percent_double;
    const std::string pair_name = pair->GetString();
    const std::string percent_change = change->GetString();
    const std::string last_price = last->GetString();

    if (!base::StringToDouble(change->GetString(), &percent_double)) {
      continue;
    }

    ranking_data.insert({"pair", pair_name});
    ranking_data.insert({"percentChange", percent_change});
    ranking_data.insert({"lastPrice", last_price});

    if (percent_double < 0.0) {
      losers.push_back(ranking_data);
    } else {
      gainers.push_back(ranking_data);
    }
  }

  rankings->insert({"gainers", gainers});
  rankings->insert({"losers", losers});

  return true;
}
