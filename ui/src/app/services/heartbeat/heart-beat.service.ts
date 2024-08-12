import {Injectable, NgZone} from '@angular/core';
import {BehaviorSubject, flatMap, interval, last, map, merge, mergeAll, Observable, Subject, timeout} from "rxjs";
import {ReceiverId} from "../../models/receiver";
import {generateReceiverName, TagUpdatesService} from "../tagUpdates/tag-updates.service";


export interface HeartBeat {
  receiver: ReceiverId,
  serial: number
}

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
    private tagUpdates: TagUpdatesService
  ) {
    this.ngZone.runOutsideAngular(() => {
      interval(3 * 1000).pipe(
        flatMap(() => Array.from({length: 3}, _ => this.generateHeartBeat()))
      )
        .subscribe(this.heartBeats)
    })

    this.heartBeats.subscribe(hb => {
      this.hbCountDown(hb.receiver)
      const s = this.activeReceivers.value.add(hb.receiver)
      console.log(s.values())
      this.ngZone.run(() => this.activeReceivers.next(s))

    })

    const allUpdates: Observable<HeartBeat> = merge(
      this.heartBeats,
      this.tagUpdates.updates.pipe(
        flatMap(updates => updates),
      )
    )

    allUpdates.subscribe(({receiver, serial}) => {
      const curr = this.messageCounter.value.get(receiver) ?? {got: 0, should: 1, last: serial}
      curr.should += serial - curr.last
      curr.last = serial
      curr.got++
      this.messageCounter.next(this.messageCounter.value.set(receiver, curr))
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
        const active = this.activeReceivers.value
        active.delete(receiver)
        this.ngZone.run(() => {
          this.activeReceivers.next(active)
        })
      }, 1e4) as number
      this.timeoutMap.set(receiver, newTimeout)
    })

  }

  generateHeartBeat(): HeartBeat {
    const uuid = generateReceiverName()
    const serial = getSerial(uuid)
    return {
      receiver: uuid, serial
    }
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
