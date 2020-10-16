/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */
import * as React from 'react'
import { ThemeProvider } from 'styled-components'

import createWidget from '../widget/index'
import { StyledTitleTab } from '../widgetTitleTab'

import {
  currencyNames,
  // dynamicBuyLink,
  links
} from './data'

import {
  ActionAnchor,
  ActionButton,
  BackArrow,
  BasicBox,
  Box,
  CryptoDotComIcon,
  FlexItem,
  Filters,
  FilterOption,
  Header,
  List,
  ListItem,
  PlainButton,
  StyledTitle,
  StyledTitleText,
  SVG,
  Text,
  WidgetWrapper,
  UpperCaseText
} from './style'
import CryptoDotComLogo from './assets/cryptoDotCom-logo'
import { CaratLeftIcon } from 'brave-ui/components/icons'
import icons from './assets/icons'

// Utils
import { getLocale } from '../../../../common/locale'

interface TickerPrice {
  price: number
  volume: number
}

interface AssetRanking {
  lastPrice: number
  pair: string
  percentChange: number
}

interface ChartDataPoint {
  c: number
  h: number
  l: number
  o: number
  t: number
  v: number
}

enum MainViews {
  TOP,
  TRADE,
  EVENTS,
  BALANCE
}

enum AssetViews {
  DETAILS,
  TRADE,
  DEPOSIT
}

interface State {
  currentView: MainViews
  currentAssetView: AssetViews
  selectedAsset: string
}

interface Props {
  showContent: boolean
  optInTotal: boolean
  optInBTCPrice: boolean
  optInMarkets: boolean
  tickerPrices: Record<string, TickerPrice>
  losersGainers: Record<string, AssetRanking[]>
  supportedPairs: Record<string, string[]>
  tradingPairs: Array<Record<string, string>>
  charts: Record<string, ChartDataPoint[]>
  stackPosition: number
  onShowContent: () => void
  onDisableWidget: () => void
  onBtcPriceOptIn: () => void
  onUpdateActions: () => Promise<void>
  onViewMarketsRequested: (assets: string[]) => void
  onAssetsDetailsRequested: (assets: string[]) => void
  onBuyCrypto: () => void
  onOptInMarkets: (show: boolean) => void
}
interface ChartConfig {
  data: Array<any>
  chartHeight: number
  chartWidth: number
}

class CryptoDotCom extends React.PureComponent<Props, State> {
  private refreshInterval: any
  private topMovers: string[] = Object.keys(currencyNames)

  constructor (props: Props) {
    super(props)
    this.state = {
      currentView: MainViews.TOP,
      currentAssetView: AssetViews.TRADE, // TODO: change back to DETAILS
      selectedAsset: ''
    }
  }

  componentDidMount () {
    const { optInBTCPrice, optInMarkets } = this.props

    if (optInBTCPrice || optInMarkets) {
      this.checkSetRefreshInterval()
    }
  }

  componentWillUnmount () {
    this.clearIntervals()
  }

  checkSetRefreshInterval = () => {
    if (!this.refreshInterval) {
      this.refreshInterval = setInterval(async () => {
        await this.props.onUpdateActions()
          .catch((_e) => console.debug('Could not update crypto.com data'))
      }, 30000)
    }
  }

  clearIntervals = () => {
    clearInterval(this.refreshInterval)
    this.refreshInterval = null
  }

  setMainView = (view: MainViews) => {
    this.setState({
      currentView: view
    })
  }

  setSelectedAsset = (asset: string) => {
    this.setState({
      selectedAsset: asset
    })
  }

  handleViewMarketsClick = () => {
    this.props.onViewMarketsRequested(this.topMovers)
  }

  optInMarkets = (show: boolean) => {
    if (show) {
      this.checkSetRefreshInterval()
    } else {
      if (!this.props.optInBTCPrice) {
        this.clearIntervals()
      }
      this.setState({ selectedAsset: '' })
    }

    this.props.onOptInMarkets(show)
  }

  btcPriceOptIn = () => {
    this.props.onBtcPriceOptIn()
    this.checkSetRefreshInterval()
  }

  handleAssetDetailClick = async (asset: string) => {
    this.setSelectedAsset(asset)
    await this.props.onAssetsDetailsRequested([asset])
  }

  onClickBuyTopDetail = () => {
    window.open(links.buyTopDetail, '_blank', 'noopener')
    this.props.onBuyCrypto()
  }

  onClickBuyTop = () => {
    window.open(links.buyTop, '_blank', 'noopener')
    this.props.onBuyCrypto()
  }

  onClickBuyBottom = () => {
    window.open(links.buyBottom, '_blank', 'noopener')
    this.props.onBuyCrypto()
  }

  onClickBuyPair = (pair: string) => {
    // TODO: delete code below?
    // window.open(dynamicBuyLink(pair), '_blank', 'noopener')
    // this.props.onBuyCrypto()

    this.setState({
      currentAssetView: AssetViews.TRADE
    })
  }

  plotData ({ data, chartHeight, chartWidth }: ChartConfig) {
    const pointsPerDay = 4
    const daysInrange = 7
    const yHighs = data.map((point: ChartDataPoint) => point.h)
    const yLows = data.map((point: ChartDataPoint) => point.l)
    const dataPoints = data.map((point: ChartDataPoint) => point.c)
    const chartAreaY = chartHeight - 2
    const max = Math.max(...yHighs)
    const min = Math.min(...yLows)
    const pixelsPerPoint = (max - min) / chartAreaY
    return dataPoints
      .map((v, i) => {
        const y = (v - min) / pixelsPerPoint
        const x = i * (chartWidth / (pointsPerDay * daysInrange))
        return `${x},${chartAreaY - y}`
      })
      .join('\n')
  }

  renderPreOptIn () {
    const { optInBTCPrice } = this.props
    const currency = 'BTC'
    const { price = null } = this.props.tickerPrices[currency] || {}

    const losersGainers = transformLosersGainers(this.props.losersGainers || {})
    const { percentChange = null } = losersGainers[currency] || {}
    return (
      <>
        <Box isFlex={true} $height={48} hasPadding={true}>
          <FlexItem $pr={5}>
            {renderIconAsset(currency.toLowerCase())}
          </FlexItem>
          <FlexItem>
              <Text>{currency}</Text>
              <Text small={true} textColor='light'>{currencyNames[currency]}</Text>
          </FlexItem>
          <FlexItem textAlign='right' flex={1}>
            {optInBTCPrice ? (
              <>
                {(price !== null) && <Text>{formattedNum(price)}</Text>}
                {(percentChange !== null) && <Text textColor={percentChange > 0 ? 'green' : 'red'}>{percentChange}%</Text>}
              </>
            ) : (
              <PlainButton onClick={this.btcPriceOptIn} textColor='green' inline={true}>
                {getLocale('cryptoDotComWidgetShowPrice')}
              </PlainButton>
            )}
          </FlexItem>
          <FlexItem $pl={5}>
            <ActionButton onClick={this.onClickBuyTop} small={true} light={true}>
              {getLocale('cryptoDotComWidgetBuy')}
            </ActionButton>
          </FlexItem>
        </Box>
        <Text center={true} $p='1em 0 0.5em' $fontSize={15}>
          {getLocale('cryptoDotComWidgetCopyOne')}
        </Text>
        <Text center={true} $fontSize={15}>
          {getLocale('cryptoDotComWidgetCopyTwo')}
        </Text>
        <ActionAnchor onClick={this.onClickBuyBottom}>
          {getLocale('cryptoDotComWidgetBuyBtc')}
        </ActionAnchor>
        <PlainButton textColor='light' onClick={this.handleViewMarketsClick} $m='0 auto'>
          {getLocale('cryptoDotComWidgetViewMarkets')}
        </PlainButton>
      </>
    )
  }

  renderAssetDetail () {
    const { selectedAsset: currency } = this.state
    const { price = null, volume = null } = this.props.tickerPrices[currency] || {}
    const chartData = this.props.charts[currency] || []
    const pairs = this.props.supportedPairs[currency] || []

    const losersGainers = transformLosersGainers(this.props.losersGainers || {})
    const { percentChange = null } = losersGainers[currency] || {}

    const chartHeight = 100
    const chartWidth = 309
    return (
      <Box hasPadding={false}>
        <FlexItem
          hasPadding={true}
          isFlex={true}
          isFullWidth={true}
          hasBorder={true}
        >
          <FlexItem>
            <BackArrow>
              <CaratLeftIcon onClick={this.setSelectedAsset.bind(this, '')} />
            </BackArrow>
          </FlexItem>
          <FlexItem $pr={5}>
            {renderIconAsset(currency.toLowerCase())}
          </FlexItem>
          <FlexItem flex={1}>
            <Text>{currency}</Text>
            <Text small={true} textColor='light'>
              {currencyNames[currency]}
            </Text>
          </FlexItem>
          <FlexItem $pl={5}>
            <ActionButton onClick={this.onClickBuyTopDetail} small={true} light={true}>
              <UpperCaseText>
                {getLocale('cryptoDotComWidgetBuy')}
              </UpperCaseText>
            </ActionButton>
          </FlexItem>
        </FlexItem>
        <FlexItem
          hasPadding={true}
          isFullWidth={true}
          hasBorder={true}
        >
          {(price !== null) && <Text
            inline={true}
            large={true}
            weight={500}
            $mr='0.5rem'
          >
            {formattedNum(price)} USDT
          </Text>}
          {(percentChange !== null) && <Text inline={true} textColor={percentChange > 0 ? 'green' : 'red'}>{percentChange}%</Text>}
          <SVG viewBox={`0 0 ${chartWidth} ${chartHeight}`}>
            <polyline
              fill='none'
              stroke='#44B0FF'
              strokeWidth='3'
              points={this.plotData({
                data: chartData,
                chartHeight,
                chartWidth
              })}
            />
          </SVG>
        <Text small={true} textColor='xlight'>
          {getLocale('cryptoDotComWidgetGraph')}
        </Text>
        </FlexItem>
        <FlexItem
          hasPadding={true}
          isFullWidth={true}
        >
          <BasicBox $mt='0.2em'>
            <Text small={true} textColor='light' $pb='0.2rem'>
              <UpperCaseText>
                {getLocale('cryptoDotComWidgetVolume')}
              </UpperCaseText>
            </Text>
            {volume && <Text weight={500}>{formattedNum(volume)} USDT</Text>}
          </BasicBox>
          <BasicBox $mt='1em'>
            <Text small={true} textColor='light' $pb='0.2rem'>
              <UpperCaseText>
                {getLocale('cryptoDotComWidgetPairs')}
              </UpperCaseText>
            </Text>
            {pairs.map((pair, i) => {
              const pairName = pair.replace('_', '/')
              return (
                <ActionButton onClick={this.onClickBuyPair.bind(this, pairName)} key={pair} small={true} inline={true} $mr={i === 0 ? 5 : 0} $mb={5}>
                  {pairName}
                </ActionButton>
              )
            })}
          </BasicBox>
        </FlexItem>
      </Box>
    )
  }

  renderAssetTrade () {
    const { selectedAsset: currency } = this.state
    const { price = null, volume = null } = this.props.tickerPrices[currency] || {}
    const chartData = this.props.charts[currency] || []
    const pairs = this.props.supportedPairs[currency] || []

    const losersGainers = transformLosersGainers(this.props.losersGainers || {})
    const { percentChange = null } = losersGainers[currency] || {}

    const chartHeight = 100
    const chartWidth = 309
    return (
      <Box hasPadding={false}>
        <FlexItem
          hasPadding={true}
          isFlex={true}
          isFullWidth={true}
          hasBorder={true}
        >
          <FlexItem>
            <BackArrow>
              <CaratLeftIcon onClick={this.setSelectedAsset.bind(this, '')} />
            </BackArrow>
          </FlexItem>
          <FlexItem $pr={5}>
            {renderIconAsset(currency.toLowerCase())}
          </FlexItem>
          <FlexItem flex={1}>
            <Text>{currency}</Text>
            <Text small={true} textColor='light'>
              {currencyNames[currency]}
            </Text>
          </FlexItem>
          <FlexItem $pl={5}>
            <ActionButton onClick={this.onClickBuyTopDetail} small={true} light={true}>
              <Filters>
                <FilterOption>Buy</FilterOption>
                <FilterOption>Sell</FilterOption>
              </Filters>
            </ActionButton>
          </FlexItem>
        </FlexItem>
        <FlexItem
          hasPadding={true}
          isFullWidth={true}
          hasBorder={true}
        >
          {(price !== null) && <Text
            inline={true}
            large={true}
            weight={500}
            $mr='0.5rem'
          >
            {formattedNum(price)} USDT
          </Text>}
          {(percentChange !== null) && <Text inline={true} textColor={percentChange > 0 ? 'green' : 'red'}>{percentChange}%</Text>}
          <SVG viewBox={`0 0 ${chartWidth} ${chartHeight}`}>
            <polyline
              fill='none'
              stroke='#44B0FF'
              strokeWidth='3'
              points={this.plotData({
                data: chartData,
                chartHeight,
                chartWidth
              })}
            />
          </SVG>
        <Text small={true} textColor='xlight'>
          {getLocale('cryptoDotComWidgetGraph')}
        </Text>
        </FlexItem>
        <FlexItem
          hasPadding={true}
          isFullWidth={true}
        >
          <BasicBox $mt='0.2em'>
            <Text small={true} textColor='light' $pb='0.2rem'>
              <UpperCaseText>
                {getLocale('cryptoDotComWidgetVolume')}
              </UpperCaseText>
            </Text>
            {volume && <Text weight={500}>{formattedNum(volume)} USDT</Text>}
          </BasicBox>
          <BasicBox $mt='1em'>
            <Text small={true} textColor='light' $pb='0.2rem'>
              <UpperCaseText>
                {getLocale('cryptoDotComWidgetPairs')}
              </UpperCaseText>
            </Text>
            {pairs.map((pair, i) => {
              const pairName = pair.replace('_', '/')
              return (
                <ActionButton onClick={this.onClickBuyPair.bind(this, pairName)} key={pair} small={true} inline={true} $mr={i === 0 ? 5 : 0} $mb={5}>
                  {pairName}
                </ActionButton>
              )
            })}
          </BasicBox>
        </FlexItem>
      </Box>
    )
  }

  renderTitle () {
    const { selectedAsset } = this.state
    const { optInMarkets, showContent } = this.props
    const shouldShowBackArrow = !selectedAsset && showContent && optInMarkets

    return (
      <Header showContent={showContent}>
        <StyledTitle>
          <CryptoDotComIcon>
            <CryptoDotComLogo />
          </CryptoDotComIcon>
          <StyledTitleText>
            {'Crypto.com'}
          </StyledTitleText>
          {shouldShowBackArrow &&
            <BackArrow marketView={true}>
              <CaratLeftIcon
                onClick={this.optInMarkets.bind(this, false)}
              />
            </BackArrow>
          }
        </StyledTitle>
      </Header>
    )
  }

  renderTitleTab () {
    const { onShowContent, stackPosition } = this.props

    return (
      <StyledTitleTab onClick={onShowContent} stackPosition={stackPosition}>
        {this.renderTitle()}
      </StyledTitleTab>
    )
  }

  renderNav () {
    const { currentView } = this.state
    return (
      <BasicBox isFlex={true} justify="start" $mb={13.5}>
        <PlainButton $pl="0" weight={600} textColor={currentView === MainViews.TOP ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.TOP)}>Top</PlainButton>
        <PlainButton weight={600} textColor={currentView === MainViews.TRADE ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.TRADE)}>Trade</PlainButton>
        <PlainButton weight={600} textColor={currentView === MainViews.EVENTS ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.EVENTS)}>Events</PlainButton>
        <PlainButton weight={600} textColor={currentView === MainViews.BALANCE ? 'white' : 'light'} onClick={this.setMainView.bind(null, MainViews.BALANCE)}>Balance</PlainButton>
      </BasicBox>
    )
  }

  renderTopView () {
    const { currentView } = this.state
    if (currentView === MainViews.TOP) {
      return <TopMovers
        tickerPrices={this.props.tickerPrices}
        losersGainers={this.props.losersGainers}
        topMovers={this.topMovers}
        handleAssetDetailClick={this.handleAssetDetailClick}
      />
    }

    if (currentView === MainViews.TRADE) {
      return <Trade tradingPairs={this.props.tradingPairs} />
    }

    return <h4>Whoops... not done yet. 😬</h4> // TODO: delete
  }

  renderAssetView () {
    const { currentAssetView } = this.state
    if (currentAssetView === AssetViews.DETAILS) {
      return this.renderAssetDetail()
    }

    if (currentAssetView === AssetViews.TRADE) {
      return this.renderAssetTrade()
    }

    return <h4>Whoops... asset view not done yet. 😬</h4> // TODO: delete
  }

  renderIndex () {
    const { selectedAsset } = this.state

    if (selectedAsset) {
      return this.renderAssetView()
    } else {
      return <>
        {this.renderNav()}
        {this.renderTopView()}
      </>
    }
  }

  render () {
    const { showContent, optInMarkets } = this.props

    if (!showContent) {
      return this.renderTitleTab()
    }

    return (
      <ThemeProvider theme={{
          secondary: "rgba(15, 28, 45, 0.7)",
          primary: "#44B0FF"
        }}>
        <WidgetWrapper tabIndex={0}>
          {this.renderTitle()}
          {(optInMarkets) ? (
            this.renderIndex()
          ) : (
            this.renderPreOptIn()
          )}
        </WidgetWrapper>
      </ThemeProvider>
    )
  }
}

export const CryptoDotComWidget = createWidget(CryptoDotCom)

// Supporting Components

function TopMovers ({
  tickerPrices,
  losersGainers,
  handleAssetDetailClick
}: any) {
  enum FilterValues {
    LOSERS = 'losers',
    GAINERS = 'gainers'
  }

  const [filter, setFilter] = React.useState(FilterValues.GAINERS);

  return <>
    <Filters>
      <FilterOption onClick={() => setFilter(FilterValues.GAINERS)} isActive={filter === FilterValues.GAINERS}>Gainers</FilterOption>
      <FilterOption onClick={() => setFilter(FilterValues.LOSERS)} isActive={filter === FilterValues.LOSERS}>Losers</FilterOption>
    </Filters>
    <List>
      {losersGainers[filter].map((asset: Record<string, any>) => {
        const currency = asset.pair.split('_')[0];
        const { percentChange } = asset
        const { price = null } = tickerPrices[currency] || {}

        return (
          <ListItem key={currency} isFlex={true} onClick={() => handleAssetDetailClick(currency)} $height={48}>
            <FlexItem $pl={5} $pr={5}>
              {renderIconAsset(currency.toLowerCase())}
            </FlexItem>
            <FlexItem>
              <Text>{currency}</Text>
              <Text small={true} textColor='light'>{currencyNames[currency]}</Text>
            </FlexItem>
            <FlexItem textAlign='right' flex={1}>
              {(price !== null) && <Text>{formattedNum(price)}</Text>}
              {(percentChange !== null) && <Text textColor={percentChange > 0 ? 'green' : 'red'}>{percentChange}%</Text>}
            </FlexItem>
          </ListItem>
        )
      })}
    </List>
  </>
}

function Trade ({ tradingPairs }: any) {
  enum FilterValues {
    BTC = 'BTC',
    CRO = 'CRO',
    USDT = 'USDT'
  }

  const [filter, setFilter] = React.useState(FilterValues.BTC)

  return <>
    <Filters>
      <FilterOption onClick={() => setFilter(FilterValues.BTC)} isActive={filter === FilterValues.BTC}>BTC</FilterOption>
      <FilterOption onClick={() => setFilter(FilterValues.CRO)} isActive={filter === FilterValues.CRO}>CRO</FilterOption>
      <FilterOption onClick={() => setFilter(FilterValues.USDT)} isActive={filter === FilterValues.USDT}>USDT</FilterOption>
    </Filters>
    <List>
      {tradingPairs
        .filter((pair: Record<string, string>) => pair.quote === filter)
        .map((pair: Record<string, string>) => {
          const price = 199
          const percentChange = 1.8

          return (
            <ListItem key={pair.pair} isFlex={true} $height={48}>
              <FlexItem $pl={5} $pr={5}>
                {renderIconAsset(pair.base.toLowerCase())}
              </FlexItem>
              <FlexItem>
                <Text>{pair.base}/{pair.quote}</Text>
              </FlexItem>
              <FlexItem textAlign='right' flex={1}>
                {(price !== null) && <Text>{formattedNum(price)}</Text>}
                {(percentChange !== null) && <Text textColor={percentChange > 0 ? 'green' : 'red'}>{percentChange}%</Text>}
              </FlexItem>
            </ListItem>
          )
      })}
    </List>
  </>
}

// Utility functions
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

// This is a temporary function only necessary for MVP
// Merges losers/gainers into one table
function transformLosersGainers ({ losers = [], gainers = [] }: Record<string, AssetRanking[]>): Record<string, AssetRanking> {
  const losersGainersMerged = [ ...losers, ...gainers ]
  return losersGainersMerged.reduce((mergedTable: object, asset: AssetRanking) => {
    let { pair: assetName, ...assetRanking } = asset
    assetName = assetName.split('_')[0]

    return {
      ...mergedTable,
      [assetName]: assetRanking
    }
  }, {})
}