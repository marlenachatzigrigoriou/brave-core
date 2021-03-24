// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'

import { currencyNames } from './data'
import { AssetViews } from './types'
import {
  formattedNum,
  getPercentColor,
  renderIconAsset
} from './utils'

import {
  FlexItem,
  ButtonGroup,
  List,
  ListItem,
  PlainButton,
  Text
} from './style'

import { getLocale } from '../../../../common/locale'

// TODO(simonhong): Remove any.
interface Props {
  losersGainers: Record<string, chrome.cryptoDotCom.AssetRanking[]>
  handleAssetClick: (base: string, quote?: string, view?: AssetViews) => Promise<void>
}

export default function TopMoversView ({
  losersGainers,
  handleAssetClick
}: Props) {
  enum FilterValues {
    LOSERS = 'losers',
    GAINERS = 'gainers'
  }

  const [filter, setFilter] = React.useState(FilterValues.GAINERS);

  const sortTopMovers = (a: Record<string, any>, b: Record<string, any>) => {
    if (filter === FilterValues.GAINERS) {
      return b.percentChange - a.percentChange
    } else {
      return a.percentChange - b.percentChange
    }
  }

  return <>
    <ButtonGroup>
      <PlainButton onClick={() => setFilter(FilterValues.GAINERS)} isActive={filter === FilterValues.GAINERS}>{getLocale('cryptoDotComWidgetGainers')}</PlainButton>
      <PlainButton onClick={() => setFilter(FilterValues.LOSERS)} isActive={filter === FilterValues.LOSERS}>{getLocale('cryptoDotComWidgetLosers')}</PlainButton>
    </ButtonGroup>
    <List>
      {losersGainers && losersGainers[filter] && losersGainers[filter]
        .sort(sortTopMovers)
        .map((asset: Record<string, any>) => {
          const currency = asset.pair.split('_')[0];
          const { percentChange, lastPrice: price } = asset

          return (
            <ListItem key={currency} isFlex={true} onClick={() => handleAssetClick(currency)} $height={48}>
              <FlexItem $pl={5} $pr={5}>
                {renderIconAsset(currency.toLowerCase())}
              </FlexItem>
              <FlexItem>
                <Text>{currency}</Text>
                <Text small={true} textColor='light'>{currencyNames[currency]}</Text>
              </FlexItem>
              <FlexItem textAlign='right' flex={1}>
                {(price !== null) && <Text>{formattedNum(price)}</Text>}
                {(percentChange !== null) && <Text textColor={getPercentColor(percentChange)}>{percentChange}%</Text>}
              </FlexItem>
            </ListItem>
          )
      })}
    </List>
  </>
}
