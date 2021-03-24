// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

import icons from './assets/icons'

function renderIconAsset (key: string) {
  if (!(key in icons)) {
    return null
  }

  return (
    <>
      <img width={25} src={icons[key]} />
    </>
  )
}

function formattedNum (price: number) {
  return new Intl.NumberFormat('en-US', {
    style: 'currency',
    currency: 'USD',
    currencyDisplay: 'narrowSymbol'
  }).format(price)
}

function decimalizeCurrency (currencyAvailable: string, currencyDecimals: number) {
  return Number(Number(currencyAvailable).toFixed(currencyDecimals))
}

function getPercentColor (percentChange: string) {
  const percentChangeNum = parseFloat(percentChange)
  return percentChangeNum === 0 ? 'light' : (percentChangeNum > 0 ? 'green' : 'red')
}

// This is a temporary function only necessary for MVP
// Merges losers/gainers into one table
function transformLosersGainers ({ losers = [], gainers = [] }: Record<string, chrome.cryptoDotCom.AssetRanking[]>): Record<string, chrome.cryptoDotCom.AssetRanking> {
  const losersGainersMerged = [ ...losers, ...gainers ]
  return losersGainersMerged.reduce((mergedTable: object, asset: chrome.cryptoDotCom.AssetRanking) => {
    let { pair: assetName, ...assetRanking } = asset
    assetName = assetName.split('_')[0]

    return {
      ...mergedTable,
      [assetName]: assetRanking
    }
  }, {})
}

export {
  formattedNum,
  decimalizeCurrency,
  getPercentColor,
  renderIconAsset,
  transformLosersGainers
}
