import {Injectable, NgZone} from '@angular/core';
import {Receiver, ReceiverId} from "../../models/receiver";
import {BeaconId} from "../../models/beacon";
import {TagUpdate} from "../../models/TagUpdate";
import {TagUpdatesService} from "../tagUpdates/tag-updates.service";
import {BehaviorSubject, distinctUntilChanged, flatMap, map} from "rxjs";
import {HeartBeatService} from "../heartbeat/heart-beat.service";


export type State = Record<ReceiverId, Receiver>


@Injectable({
  providedIn: 'root'
})
export class GridStateService {
  state = new BehaviorSubject<State>({})
  lastUpdate = new Map<ReceiverId, number>()
  likelyIn = new BehaviorSubject<Record<BeaconId, Receiver>>({})
  beacons = new BehaviorSubject<BeaconId[]>([])
  receivers = new BehaviorSubject<ReceiverId[]>([])


  constructor(
    private tagUpdates: TagUpdatesService,
    private zone: NgZone,
    private heartBeatService: HeartBeatService
  ) {
    this.state.pipe(
      map(x => Object.keys(x)),
      distinctUntilChanged(),
    ).subscribe(names => this.receivers.next(Array.from(new Set(names))))

    this.state.pipe(
      map(state => Object.entries(state)),
      map((recs, idx) =>
        recs.flatMap(([recName, rec]) => Object.keys(rec.beacons))
      ),
      distinctUntilChanged()
    ).subscribe(names => {
      this.beacons.next(Array.from(new Set(names)))
    })

    this.state.pipe(map(this.toLikelyInMap.bind(this))).subscribe(this.likelyIn)

    this.tagUpdates.updates.subscribe(this.handleTagUpdate.bind(this))
  }

  handleTagUpdate(updates: TagUpdate[]) {
    let state = this.state.value

    for (const tag of updates) {
      this.lastUpdate.set(tag.receiver, Date.now())

      let receiver = state[tag.receiver] ?? {
        beacons: {}, uuid: tag.receiver
      }

      receiver.beacons[tag.beacon.ID] = tag.beacon

      const entry: Record<ReceiverId, Receiver> = {}
      entry[tag.receiver] = receiver


      state = {...state, ...entry}
    }


    this.zone.run(() => {
      this.state.next(state)
    })
  }

  clear() {
    this.state.next({})
    this.likelyIn.next({})
    this.lastUpdate.clear()
    this.heartBeatService.clear()
  }

  beaconReceiverPairs(rec: Receiver): [BeaconId, ReceiverId][] {
    return Object.keys(rec.beacons).map(beacon => [beacon, rec.uuid])
  }


  toLikelyInMap(state: State) {
    const result: Record<BeaconId, Receiver> = {}

    for (const receiver of Object.values(state)) {
      const beacons = Object.values(receiver.beacons)
      for (let beacon of beacons) {
        const curr = result[beacon.ID];
        if (!curr) {
          result[beacon.ID] = receiver
        } else if (curr.beacons[beacon.ID]?.rssi ?? 1e200 >
          beacon.rssi) {
          result[beacon.ID] = receiver
        }
      }
    }
    return result
  }
}

function groupBy<T, V>(arr: T[], key: (item: T) => string | number, value: (item: T) => V) {
  const result: Record<string | number, V[]> = {}

  for (const item of arr) {
    const itemKey = key(item)
    const itemValue = value(item)

    if (!result[itemKey]) {
      result[itemKey] = []
    }

    result[itemKey].push(itemValue)
  }

  return result
}
