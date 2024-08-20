import {Injectable, NgZone} from '@angular/core';
import {BehaviorSubject, flatMap, interval, last, map, merge, mergeAll, Observable, Subject, timeout} from "rxjs";
import {ReceiverId} from "../../models/receiver";
import {TagUpdatesService} from "../tagUpdates/tag-updates.service";
import {HeartBeat} from "../../models/heartBeat";
import {TagMessage} from "../../models/TagMessage";
import {SocketIoService} from "../socket-io.service";


export type MessageCount = { should: number, got: number, last: number }

@Injectable({
  providedIn: 'root'
})
export class HeartBeatService {
  heartBeats = new Subject<HeartBeat>()
  activeReceivers = new BehaviorSubject<Set<ReceiverId>>(new Set())
  messageCounter = new BehaviorSubject<Map<ReceiverId, MessageCount>>(new Map())
  timeoutMap = new Map<ReceiverId, number>()


  constructor(
    private ngZone: NgZone,
    private tagUpdates: TagUpdatesService,
    private socketIo: SocketIoService
  ) {
    setInterval(() => {
      console.log(`Active receives are ${this.activeReceivers.value.values()}`)
    }, 3000)

    this.socketIo.SubscribeToEvent<HeartBeat>('heartbeat_message', ((data: HeartBeat) => this.heartBeats.next(data)));

    this.heartBeats.subscribe(hb => {
      this.hbCountDown(hb.uuid)
      const newActive = new Set(this.activeReceivers.value);
      newActive.add(hb.uuid);
      console.log('Active receivers:', Array.from(newActive));
      this.activeReceivers.next(newActive);

    })

    const allUpdates: Observable<HeartBeat | TagMessage> = merge(
      this.heartBeats,
      this.tagUpdates.updates)

    allUpdates.subscribe(({uuid, msg_counter}) => {
      let curr = this.messageCounter.value.get(uuid) ?? {got: 0, should: 1, last: msg_counter}
      if (curr.last > msg_counter) {
        this.messageCounter.next(new Map())
        curr = {got: 0, should: 1, last: msg_counter}
      }


      curr.should += msg_counter - curr.last
      curr.last = msg_counter
      curr.got++
      this.messageCounter.next(this.messageCounter.value.set(uuid, curr))
    })
  }

  hbCountDown(receiver: ReceiverId) {
    this.ngZone.runOutsideAngular(() => {
      const timeout = this.timeoutMap.get(receiver)
      if (timeout !== undefined) {
        clearTimeout(timeout)
        this.timeoutMap.delete(receiver)
      }
      const newTimeout = setTimeout(() => {
        console.log('lost', receiver)
        this.timeoutMap.delete(receiver)
        const active = new Set(this.activeReceivers.value);
        active.delete(receiver)
        this.ngZone.run(() => {
          this.activeReceivers.next(active)
        })
      }, 1e4) as number
      this.timeoutMap.set(receiver, newTimeout)
    })
  }

  clear() {
    this.activeReceivers.next(new Set())
    this.messageCounter.next(new Map())
    this.timeoutMap.forEach((timer, recId) => {
      clearTimeout(timer)
    })
    this.timeoutMap.clear()
  }
}

const serialMap = new Map<ReceiverId, number>()

export function getSerial(receiver: ReceiverId) {
  const serial = (serialMap.get(receiver) ?? 0) + 1
  serialMap.set(receiver, serial)
  return serial
}
