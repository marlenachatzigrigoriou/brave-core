// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { CaratLeftIcon } from 'brave-ui/components/icons'

import { currencyNames } from './data'
import {
  formattedNum,
  decimalizeCurrency,
  renderIconAsset
} from './utils'
import { getLocale } from '../../../../common/locale'

import {
  ActionButton,
  AmountInputField,
  BackArrow,
  BasicBox,
  Box,
  FlexItem,
  ButtonGroup,
  PlainButton,
  Text
} from './style'

interface Props {
  base : string
  quote: string
  availableBalanceBase: number
  availableBalanceQuote: number
  priceDecimals: string
  quantityDecimals: string
  tickerPrices: Record<string, chrome.cryptoDotCom.TickerPrice>
  handleBackClick: () => void
}

export default function AssetTradeView ({
  base,
  quote,
  availableBalanceBase,
  availableBalanceQuote,
  handleBackClick,
  tickerPrices,
  priceDecimals,
  quantityDecimals,
}: Props) {

  enum TradeModes {
    BUY = 'buy',
    SELL = 'sell'
  }

  const tradeModeLocaleStrings = {
    buy: getLocale('cryptoDotComWidgetBuy'),
    sell: getLocale('cryptoDotComWidgetSell')
  }

  enum Percentages {
    twentyFive = 25,
    fifty = 50,
    seventyFive = 75,
    onehundred = 100
  }

  const confirmDelay = 15

  const [tradeMode, setTradeMode] = React.useState(TradeModes.BUY)
  const [tradePercentage, setTradePercentage] = React.useState<number | null>(null)
  const [tradeAmount, setTradeAmount] = React.useState('')
  const [showConfirmScreen, setConfirmScreen] = React.useState(false)
  const [counter, setCounter] = React.useState(confirmDelay)

  let timerId: any;
  const { price: unitPrice = 0 } = tickerPrices[`${base}_${quote}`] || {}
  const approxTotal = Number(tradeAmount) * unitPrice

  const handlePercentageClick = (percentage: number) => {
    const availableBalance =
        (tradeMode === TradeModes.BUY ? availableBalanceQuote : availableBalanceBase)
    const amount = (percentage / 100) * availableBalance
    setTradeAmount(`${amount}`)
    setTradePercentage(percentage)
  }

  const handleAmountChange = ({ target }: any) => {
    const { value } = target
    if (value === "." || !Number.isNaN(value * 1)) {
      const available = tradeMode === TradeModes.BUY ? availableBalanceQuote
                                                     : availableBalanceBase
      // Can't put more larger amount than available.
      if (Number(available) < Number(value)) {
        return
      }
      setTradeAmount(value)
      setTradePercentage(null)
    }
  }

  const getPlaceholderText = () => tradeMode === TradeModes.BUY ? (
    getLocale('cryptoDotComWidgetTradeTo', {
      fromCurrency: quote,
      toCurrency: base
    })
  ) : (
    getLocale('cryptoDotComWidgetTradeTo', {
      fromCurrency: base,
      toCurrency: quote
    })
  )

  React.useEffect(() => {
    if (showConfirmScreen) {
      timerId = counter > 0 && setInterval(() => {
        if (counter > 0) {
          console.log(counter)
          setCounter(counter - 1)
        }
      }, 1000);
    }
    return () => clearInterval(timerId);
  }, [counter, showConfirmScreen]);

  const clearTimers = () => {
    clearTimeout(timerId)
    setCounter(confirmDelay)
  }

  const handlePurchaseClick = () => {
    setConfirmScreen(true)
  }

  const handleConfirmClick = () => {
    // Call order api.
    const order = {
      'instrument_name': `${base}_${quote}`,
      'type': 'MARKET',
    }

    if (tradeMode === TradeModes.BUY) {
      order['side'] = 'BUY'
      order['notional'] = decimalizeCurrency(approxTotal.toString(), Number(priceDecimals))
    } else {
      order['side'] = 'SELL'
      order['quantity'] = decimalizeCurrency(tradeAmount.toString(), Number(quantityDecimals))
    }

    // Do we need show order feedback?
    chrome.cryptoDotCom.createMarketOrder(order , (success: boolean) => { console.log(success)})
    clearTimers()
    setTradeAmount('')
    setConfirmScreen(false)
  }

  // Reset amount whenever changing between buy/sell.
  const handleSetTradeMode = (mode: TradeModes) => {
    setTradeMode(mode)
    setTradeAmount('')
  }

  const handleCancelClick = () => {
    clearTimers()
    setConfirmScreen(false)
  }

  const buyingString = getLocale('cryptoDotComWidgetBuying')
  const sellingString = getLocale('cryptoDotComWidgetSelling')
  return showConfirmScreen ? (
    <>
      <Box>
        <Text center={true} weight={600} $pb={15}>{getLocale('cryptoDotComWidgetConfirmOrder')}</Text>
        <BasicBox $pb={7}>
          <Text weight={600} textColor='light' $fontSize={12}>{tradeMode === TradeModes.BUY ? buyingString : sellingString}</Text>
          <Text $fontSize={16}>{tradeAmount} {base}</Text>
        </BasicBox>
        <BasicBox $pb={7}>
          <Text weight={600} textColor='light' $fontSize={12}>*{getLocale('cryptoDotComWidgetApproxPrice')}</Text>
          <Text $fontSize={16}>{quote === 'USDT' ? formattedNum(unitPrice) : unitPrice} {base}/{quote}</Text>
        </BasicBox>
        <BasicBox $pb={7}>
          {tradeMode === TradeModes.BUY ? (
            <Text weight={600} textColor='light' $fontSize={12}>*{getLocale('cryptoDotComWidgetApproxTotalSpent')}</Text>
          ) : (
            <Text weight={600} textColor='light' $fontSize={12}>*{getLocale('cryptoDotComWidgetApproxTotalReceived')}</Text>
          )}
          <Text $fontSize={16}>{quote === 'USDT' ? formattedNum(approxTotal) : approxTotal} {quote}</Text>
        </BasicBox>
        <Text textColor='light' $fontSize={12}>* {getLocale('cryptoDotComWidgetApproxFootnote')}</Text>
      </Box>
      <BasicBox $pt={15}>
        <ActionButton onClick={handleConfirmClick}>{getLocale('cryptoDotComWidgetConfirm')} ({counter}s)</ActionButton>
        <PlainButton $pb={5} onClick={handleCancelClick} $pt={10} $m='0 auto' textColor='light'>{getLocale('cryptoDotComWidgetCancel')}</PlainButton>
      </BasicBox>
    </>
  ) : (
    <Box $p={0}>
      <FlexItem
        hasPadding={true}
        isFlex={true}
        isFullWidth={true}
        hasBorder={true}
      >
        <FlexItem>
          <BackArrow onClick={handleBackClick}>
            <CaratLeftIcon />
          </BackArrow>
        </FlexItem>
        <FlexItem $pr={5}>
          {renderIconAsset(base.toLowerCase())}
        </FlexItem>
        <FlexItem flex={1}>
          <Text>{base}</Text>
          <Text small={true} textColor='light'>
            {currencyNames[base]}
          </Text>
        </FlexItem>
        <FlexItem $pl={5}>
          <ButtonGroup>
            <PlainButton onClick={() => handleSetTradeMode(TradeModes.BUY)} textColor='green'>Buy</PlainButton>
            <PlainButton onClick={() => handleSetTradeMode(TradeModes.SELL)} textColor='red'>Sell</PlainButton>
          </ButtonGroup>
        </FlexItem>
      </FlexItem>
      <FlexItem
        hasPadding={true}
        isFullWidth={true}
        hasBorder={true}
      >
        {tradeMode === TradeModes.BUY ? (
          <Text $mt={15} center={true}>{availableBalanceQuote} {quote} {getLocale('cryptoDotComWidgetAvailable')}</Text>
        ) : (
          <Text $mt={15} center={true}>{availableBalanceBase} {base} {getLocale('cryptoDotComWidgetAvailable')}</Text>
        )}
        <AmountInputField
          $mt={10} $mb={10}
          placeholder={getPlaceholderText()}
          onChange={handleAmountChange}
          value={tradeAmount}
        />
        <BasicBox isFlex={true} justify="center" $mb={13.5}>
          {Object.values(Percentages).map(percentage => {
            return (typeof percentage === 'number') && (
              <PlainButton
                key={percentage}
                weight={500}
                textColor={tradePercentage === percentage ? 'white' : 'light'}
                onClick={() => handlePercentageClick(percentage)}
              >
                {percentage}%
              </PlainButton>
            )
          })}
        </BasicBox>
      </FlexItem>
      <FlexItem
        hasPadding={true}
        isFullWidth={true}
      >
        <ActionButton onClick={handlePurchaseClick} disabled={!tradeAmount} textOpacity={tradeAmount ? 1 : 0.6} $bg={tradeMode === TradeModes.BUY ? 'green' : 'red-bright'} upperCase={false}>{tradeModeLocaleStrings[tradeMode]} {base}</ActionButton>
      </FlexItem>
    </Box>
  )
}
