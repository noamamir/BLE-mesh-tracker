import {Injectable, NgZone} from '@angular/core';
import {Receiver, ReceiverId} from "../../models/receiver";
import {Beacon, BeaconId} from "../../models/beacon";
import {TagMessage} from "../../models/TagMessage";
import {TagUpdatesService} from "../tagUpdates/tag-updates.service";
import {BehaviorSubject, distinctUntilChanged, flatMap, map, Observable} from "rxjs";
import {HeartBeatService} from "../heartbeat/heart-beat.service";
import {HttpClient} from "@angular/common/http";
import {SocketIoService} from "../socket-io.service";
import {HeartBeat} from "../../models/heartBeat";


export type State = Record<ReceiverId, Receiver>


@Injectable({
  providedIn: 'root'
})
export class GridStateService {

  private apiUrl = 'http://localhost:5000'; // Update with your server URL

  constructor(
    private http: HttpClient,
    private tagUpdates: TagUpdatesService,
    private zone: NgZone,
    private heartBeatService: HeartBeatService,
  ) {
    this.listenForUpdates();

    this.state.pipe(
      distinctUntilChanged(),
    ).subscribe(state => {
      const ids = Object.keys(state);
      this.receivers.next(Array.from(new Set(ids)))
    })

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
  }

  // private initializeReceivers(): void {
  //   this.http.get<Record<ReceiverId, Receiver>>(`${this.apiUrl}/devices`).subscribe(
  //     (receivers) => {
  //
  //       this.receiversSubject.next(receivers);
  //     },
  //     (error) => console.error('Error fetching receivers:', error)
  //   );
  // }

  private listenForUpdates(): void {
    this.tagUpdates.updates.subscribe((update) => this.handleTagUpdate(update))
    this.heartBeatService.heartBeats.subscribe((heartbeat) => this.updateReceiverWithHeartbeat(heartbeat))
  }


  private updateReceiverWithHeartbeat(heartbeat: HeartBeat): void {
    // Implement heartbeat update logic if needed
    console.log('Received heartbeat:', heartbeat);
  }


  syncTime(): Observable<boolean> {
    return this.http.post<boolean>(`${this.apiUrl}/syncTime`, {});
  }

  state = new BehaviorSubject<State>({})
  lastUpdate = new Map<ReceiverId, number>()
  likelyIn = new BehaviorSubject<Record<BeaconId, Receiver>>({})
  beacons = new BehaviorSubject<BeaconId[]>([])
  receivers = new BehaviorSubject<ReceiverId[]>([])


  handleTagUpdate(tag: TagMessage) {
    let state = this.state.value;

    let receiver = state[tag.uuid] ?? {id: tag.uuid, beacons: {}}

    const time = Date.now()
    receiver.beacons[tag.addr] = new Beacon(tag.addr, tag.rssi, time);

    const entry: Record<ReceiverId, Receiver> = {}
    entry[tag.uuid] = receiver

    state = {...state, ...entry}

    this.zone.run(() => {
      this.state.next(state);
    })
  }

  clear() {
    this.state.next({})
    this.likelyIn.next({})
    this.lastUpdate.clear()
    this.heartBeatService.clear()
  }


  toLikelyInMap(state: State) {
    const result: Record<BeaconId, Receiver> = {}
    const timeNow = Date.now()
    for (const receiver of Object.values(state)) {
      const beacons = Object.values(receiver.beacons)
      for (let beacon of beacons) {
        // filter out inactive beacons
        if (timeNow - beacon.lastUpdated < 30 * 1e3) {
          const curr = result[beacon.ID];

          if (!curr) {
            result[beacon.ID] = receiver
          } else if (curr.beacons[beacon.ID].rssi > beacon.rssi) {
            result[beacon.ID] = receiver
          }
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
